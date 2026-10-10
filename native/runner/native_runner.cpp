// Copyright 2026 Lucas Mirelmann

#include "native/runner/native_runner.hpp"

#include <cassert>

#include <format>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <functional>
#include <utility>

#include "compiler/compiler.hpp"
#include "errors/source_highlight.hpp"
#include "google/protobuf/arena.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "logging/logging.hpp"
#include "native/engine/cache.hpp"
#include "native/engine/orc_engine.hpp"
#include "native/exec/exec_error.hpp"
#include "native/exec/native_exec_context.hpp"
#include "native/exec/object_runtime.hpp"
#include "native/exec/runtime_shim.hpp"
#include "native/ir/llvm_ir_generator.hpp"
#include "native/exec/module_runtime_state.hpp"
#include "proto/starlark_bytecode.pb.h"
#include "runtime/error_fn.hpp"
#include "vm/module_load_tracker.hpp"
#include "vm/module_metadata.hpp"
#include "vm/program_limits.hpp"

using ::starlark::bytecode::Program;
using ::starlark::compiler::compiler;
using ::starlark::grammar::grammar_options;
using ::starlark::logging::LogLevel;
using ::starlark::logging::Position;
using ::starlark::logging::logger;
using ::starlark::result::status_code;
using ::starlark::result::status_or;
using ::starlark::runtime::runtime_options;
using ::starlark::vm::frame;
using ::starlark::vm::module_loader;

namespace starlark {
namespace native {

namespace {

std::string target_triple() {
  return llvm::sys::getDefaultTargetTriple();
}

struct module_load_context {
  starlark::vm::module_load_tracker tracker;
  native_runner_state* runner_state = nullptr;
};

class native_error_handler : public starlark::runtime::error_fn {
 public:
  native_error_handler(logger& log, std::string_view module, module_runtime_state* state) :
      log_(log), module_(module), state_(state) {}

  void add_error(std::string_view error_msg) override {
    add_error(error_msg, Position::default_instance(), "");
  }

  void add_error(std::string_view error_msg, std::string_view hint) override {
    add_error(error_msg, Position::default_instance(), hint);
  }

  void add_error(std::string_view error_msg, const Position& pos) override {
    add_error(error_msg, pos, "");
  }

  void add_error(std::string_view error_msg, const Position& pos, std::string_view hint) override {
    mark_failed();
    const module_runtime_state* loc = location_state();
    if (loc != nullptr && loc->program != nullptr && loc->starlark_loader != nullptr) {
      const starlark::bytecode::OpCode* op = nullptr;
      if (try_error_opcode(loc, &op) && op->has_sh()) {
        auto mod = loc->starlark_loader->load_module(loc->module_name);
        if (mod.ok()) {
          auto msg = std::format("{}\n{}",
              error_msg,
              starlark::error_messages::get_line_and_underline((*mod)->source_code(),
                  op->sh().start(),
                  op->sh().highlight_start(),
                  op->sh().highlight_end(),
                  op->sh().end(),
                  hint));
          log_.log(LogLevel::LOG_LEVEL_ERROR, msg, loc->module_name, op->sh().highlight_start());
          return;
        }
      }
    }
    log_.log(LogLevel::LOG_LEVEL_ERROR, std::format("{}\n", error_msg), module_, error_position(pos));
  }

 private:
  module_runtime_state* location_state() const {
    module_runtime_state* loc = state_;
    if (state_ != nullptr && !state_->active_exec_stack.empty()) {
      if (auto* exec = state_->active_exec_stack.back()) {
        if (exec->code_mod != nullptr) {
          loc = exec->code_mod;
        } else if (exec->mod != nullptr) {
          loc = exec->mod;
        }
      }
    }
    return loc;
  }

  static bool try_error_opcode(const module_runtime_state* loc, const starlark::bytecode::OpCode** out_op) {
    *out_op = nullptr;
    if (loc == nullptr || loc->program == nullptr) {
      return false;
    }
    if (loc->error_block_ptr < 0 || loc->error_block_ptr >= loc->program->block_size()) {
      return false;
    }
    const auto& block = loc->program->block(loc->error_block_ptr);
    if (loc->error_ip < 0 || loc->error_ip >= block.op_code().size()) {
      return false;
    }
    *out_op = &block.op_code(loc->error_ip);
    return true;
  }

  Position error_position(const Position& provided = Position::default_instance()) const {
    if (provided.has_row()) {
      return provided;
    }
    const module_runtime_state* loc = location_state();
    const starlark::bytecode::OpCode* op = nullptr;
    if (try_error_opcode(loc, &op) && op->has_sh()) {
      return op->sh().highlight_start();
    }
    return Position::default_instance();
  }

  void mark_failed() {
    mark_module_failed(state_);
  }

  logger& log_;
  std::string module_;
  module_runtime_state* state_;
};

status_or<frame*> init_cached_module(native_runtime& runtime,
    module_runtime_state& state,
    starlark::vm::module_info* mod_info,
    module_loader& loader,
    const runtime_options& r_options,
    module_load_context& load_ctx,
    logger& logging,
    bool rerun) {
  state.starlark_loader = &loader;
  state.runtime_options = &r_options;
  if (state.exec_context != nullptr && load_ctx.runner_state != nullptr) {
    state.exec_context->runner_context = load_ctx.runner_state;
  }

  if (rerun) {
    state.prepare_rerun();
  } else if (state.init_done && state.compatibility_frame != nullptr) {
    if (!mod_info->ready()) {
      mod_info->loaded(state.compatibility_frame, state.program);
    }
    runtime.reset_transient_state();
    return status_or<frame*>(mod_info->get().first);
  }

  const starlark_module_descriptor* descriptor = runtime.jit().find_loaded(state.cache_key);
  if (descriptor == nullptr) {
    logging.log(LogLevel::LOG_LEVEL_ERROR,
        std::format("native module cache key {:x} is missing JIT code", state.cache_key),
        mod_info->cannonical_name(),
        Position::default_instance());
    return status_or<frame*>(status_code::kStaticError);
  }

  native_error_handler err(logging, mod_info->cannonical_name(), &state);
  if (descriptor->init != nullptr && !descriptor->init(&state, err)) {
    return status_or<frame*>(status_code::kRuntimeError);
  }

  if (!mod_info->ready()) {
    mod_info->loaded(state.compatibility_frame, state.program);
  }
  runtime.reset_transient_state();
  return status_or<frame*>(mod_info->get().first);
}

status_or<frame*> run_impl(native_runtime& runtime,
    module_loader& loader,
    std::string_view module_name,
    const grammar_options& g_options,
    const runtime_options& r_options,
    const native_options& n_options,
    logger& logging,
    module_load_context& load_ctx,
    std::string_view caller_module_name) {
  retain_runtime_symbols_for_jit();
  retain_object_runtime_symbols_for_jit();
  auto mod_info = loader.load_module(module_name, caller_module_name);
  if (!mod_info.ok()) {
    return status_or<frame*>(status_code::kStaticError);
  }

  if ((*mod_info)->ready()) {
    load_ctx.tracker.module_reduction = true;
    return status_or<frame*>((*mod_info)->get().first);
  }

  if (load_ctx.runner_state != nullptr && caller_module_name.empty()) {
    load_ctx.runner_state->has_main_module_cache_key = false;
  }

  std::string_view c_name = (*mod_info)->cannonical_name();
  auto module_processing_it = load_ctx.tracker.module_processing.find(std::string{c_name});
  if (module_processing_it != load_ctx.tracker.module_processing.end() &&
      (!load_ctx.tracker.module_reduction || module_processing_it->second + 1 != load_ctx.tracker.module_lookup.size())) {
    logging.log(LogLevel::LOG_LEVEL_ERROR,
        std::format("recursion found during module lookup\n{}",
            starlark::vm::report_recursion_in_modules(load_ctx.tracker.module_lookup, module_processing_it->second)),
        caller_module_name.empty() ? std::string{module_name} : std::string{caller_module_name},
        Position::default_instance());
    return status_or<frame*>(status_code::kStaticError);
  }
  if (module_processing_it == load_ctx.tracker.module_processing.end()) {
    load_ctx.tracker.module_processing[std::string{c_name}] = load_ctx.tracker.module_lookup.size();
    load_ctx.tracker.module_lookup.push_back(std::string{c_name});
  }
  load_ctx.tracker.module_reduction = false;

  std::set<std::string, std::less<>> binding;
  for (const auto& [key, value] : (*mod_info)->custom_binding()) {
    binding.insert(key);
  }
  starlark::compiler::compiler star_compiler(binding);
  const auto module_source = (*mod_info)->source_code();
  const auto module_c_name = (*mod_info)->cannonical_name();
  Program* program = const_cast<Program*>(runtime.find_bytecode(module_c_name, module_source));
  std::unique_ptr<google::protobuf::Arena> bytecode_arena;
  if (program == nullptr) {
    bytecode_arena = std::make_unique<google::protobuf::Arena>();
    program = star_compiler.compile(module_c_name, module_source, g_options, logging, *bytecode_arena);
  }
  if (program == nullptr) {
    return status_or<frame*>(status_code::kStaticError);
  }

  auto deps = starlark::vm::get_dependencies(program);
  for (const auto& dep : deps) {
    auto dep_result = run_impl(runtime, loader, dep, g_options, r_options, n_options, logging, load_ctx, (*mod_info)->cannonical_name());
    if (!dep_result.ok()) {
      return status_or<frame*>(status_code::kStaticError);
    }
  }

  std::string triple = target_triple();
  cache_key_parts parts{
      .module_source = (*mod_info)->source_code(),
      .dep_keys = {},
      .target_triple = triple,
  };
  for (const auto& dep : deps) {
    auto dep_info = loader.load_module(dep);
    if (dep_info.ok() && (*dep_info)->ready()) {
      parts.dep_keys.push_back(compute_cache_key({.module_source = (*dep_info)->source_code(), .dep_keys = {}, .target_triple = triple}));
    }
  }
  uint64_t cache_key = compute_cache_key(parts);

  if (load_ctx.runner_state != nullptr && caller_module_name.empty() && !load_ctx.runner_state->has_main_module_cache_key) {
    load_ctx.runner_state->main_module_cache_key = cache_key;
    load_ctx.runner_state->has_main_module_cache_key = true;
  }

  if (load_ctx.runner_state != nullptr && (*mod_info)->cannonical_name() == starlark::vm::builtin_star_module) {
    load_ctx.runner_state->builtin_module_cache_key = cache_key;
    load_ctx.runner_state->has_builtin_module_cache_key = true;
  }

  if (auto* existing = runtime.find_state(cache_key)) {
    if (bytecode_arena != nullptr) {
      runtime.store_bytecode(module_c_name, module_source, std::move(bytecode_arena), program);
    }
    existing->program = program;
    existing->starlark_loader = &loader;
    if (existing->init_done && existing->compatibility_frame != nullptr) {
      return init_cached_module(runtime, *existing, *mod_info, loader, r_options, load_ctx, logging, n_options.rerun_module);
    }
    runtime.discard_state(cache_key);
  }

  auto state_ptr = std::make_unique<module_runtime_state>();
  module_runtime_state& state_ref = *state_ptr;
  state_ref.cache_key = cache_key;
  state_ref.program = program;
  state_ref.starlark_loader = &loader;
  state_ref.runtime_options = &r_options;
  state_ref.module_name = (*mod_info)->cannonical_name();
  state_ref.inner_module = (*mod_info)->inner();

  assert(program->max_eval_stack_depth() > 0);
  state_ref.max_stack_depth = program->max_eval_stack_depth();
  state_ref.stack_buffer.resize(state_ref.max_stack_depth);
  state_ref.init_metadata = starlark::vm::module_metadata::build(*program);

  if (auto violation = starlark::vm::check_program_limits(*program, (*mod_info)->source_code(), r_options)) {
    logging.log(LogLevel::LOG_LEVEL_ERROR, violation->message, (*mod_info)->cannonical_name(), violation->position);
    return status_or<frame*>(status_code::kRuntimeError);
  }

  auto cache_root = n_options.cache_root.empty() ? default_cache_root() : std::filesystem::path{n_options.cache_root};
  const bool write_back = n_options.write_back_dylib || n_options.warm_cache;
  const starlark_module_descriptor* descriptor = nullptr;
  auto& engine = runtime.jit();

  if (auto cached = find_cached_dylib(cache_key, cache_root)) {
    auto loaded = engine.load_cached(cache_key, *cached);
    if (loaded.ok()) {
      descriptor = *loaded;
    }
  }

  if (descriptor == nullptr) {
    if (const auto* in_memory = engine.find_loaded(cache_key)) {
      descriptor = in_memory;
    }
  }

  if (descriptor == nullptr) {
    auto context = std::make_unique<llvm::LLVMContext>();
    llvm_ir_generator generator(*context);
    irgen_options options{
        .cache_key = cache_key,
        .max_stack_depth = state_ref.max_stack_depth,
        .metadata = &state_ref.init_metadata,
    };
    auto module = generator.generate(*program, options);
    if (write_back) {
      auto module_for_cache = llvm::CloneModule(*module);
      auto dylib_path = dylib_path_for_key(cache_key, cache_root);
      (void)engine.write_back(cache_key, *module_for_cache, dylib_path);
    }
    auto loaded = engine.load_jit(cache_key, std::move(module), std::move(context));
    if (!loaded.ok()) {
      return status_or<frame*>(status_code::kStaticError);
    }
    descriptor = *loaded;
  }

  module_runtime_state& state = runtime.emplace_state(cache_key, std::move(state_ptr));
  if (bytecode_arena != nullptr) {
    runtime.store_bytecode(module_c_name, module_source, std::move(bytecode_arena), program);
  }
  return init_cached_module(runtime, state, *mod_info, loader, r_options, load_ctx, logging, false);
}

}  // namespace

status_or<frame*> native_runner::run(module_loader& loader,
    std::string_view module_name,
    const grammar_options& g_options,
    const runtime_options& r_options,
    const native_options& n_options,
    logger& logging) {
  native_runner_state runner_state{
      .runtime = &runtime_,
      .loader = &loader,
      .options = &n_options,
  };
  module_load_context load_ctx;
  load_ctx.runner_state = &runner_state;
  return run_impl(runtime_, loader, module_name, g_options, r_options, n_options, logging, load_ctx, "");
}

status_or<frame*> native_runner::rerun(module_loader& loader,
    std::string_view module_name,
    const grammar_options& g_options,
    const runtime_options& r_options,
    const native_options& n_options,
    logger& logging) {
  native_options rerun_options = n_options;
  rerun_options.rerun_module = true;
  native_runner_state runner_state{
      .runtime = &runtime_,
      .loader = &loader,
      .options = &rerun_options,
  };
  module_load_context load_ctx;
  load_ctx.runner_state = &runner_state;
  return run_impl(runtime_, loader, module_name, g_options, r_options, rerun_options, logging, load_ctx, "");
}

}  // namespace native
}  // namespace starlark
