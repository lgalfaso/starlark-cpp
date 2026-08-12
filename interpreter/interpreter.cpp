// Copyright 2024-2026 Lucas Mirelmann

#include "interpreter/interpreter.hpp"

#include <iostream>

#include <functional>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "bigint/number.hpp"
#include "compiler/compiler.hpp"
#include "errors/runtime_error_messages.hpp"
#include "errors/source_highlight.hpp"
#include "interpreter/built_in_functions.hpp"
#include "interpreter/frame.hpp"
#include "interpreter/function.hpp"
#include "interpreter/runner_state.hpp"
#include "runtime/error_fn.hpp"
#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_function.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_tuple.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::google::protobuf::RepeatedPtrField;
using ::starlark::bigint::number;
using ::starlark::bigint::parse_number;
using ::starlark::bytecode::BlockType;
using ::starlark::bytecode::OpCode;
using ::starlark::bytecode::Program;
using ::starlark::compiler::compiler;
using ::starlark::error_messages::error_dictionary_duplicate_key;
using ::starlark::error_messages::error_expect_mapping_after_star_star;
using ::starlark::error_messages::error_module_does_not_define_symbol;
using ::starlark::error_messages::error_module_not_ready;
using ::starlark::error_messages::error_multiple_values_for_keyword;
using ::starlark::error_messages::error_symbol_not_available;
using ::starlark::error_messages::error_unable_to_load_module;
using ::starlark::error_messages::error_unbound_variable;
using ::starlark::error_messages::error_unknown_op;
using ::starlark::error_messages::error_v2_keyword_must_be_string;
using ::starlark::error_messages::error_v2_max_bytes_length;
using ::starlark::error_messages::error_v2_max_sequence_length;
using ::starlark::error_messages::error_v2_max_string_length;
using ::starlark::error_messages::get_line_and_underline;
using ::starlark::grammar::grammar_options;
using ::starlark::grammar::predeclared_symbols;
using ::starlark::logging::logger;
using ::starlark::result::status_code;
using ::starlark::result::status_or;
using ::starlark::runtime::context;
using ::starlark::runtime::create_function;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_built_in_functions;
using ::starlark::runtime::starlark_bytes;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_float;
using ::starlark::runtime::starlark_fn_abs;
using ::starlark::runtime::starlark_fn_all;
using ::starlark::runtime::starlark_fn_any;
using ::starlark::runtime::starlark_fn_bool;
using ::starlark::runtime::starlark_fn_bytes;
using ::starlark::runtime::starlark_fn_chr;
using ::starlark::runtime::starlark_fn_dict;
using ::starlark::runtime::starlark_fn_dir;
using ::starlark::runtime::starlark_fn_enumerate;
using ::starlark::runtime::starlark_fn_fail;
using ::starlark::runtime::starlark_fn_float;
using ::starlark::runtime::starlark_fn_getattr;
using ::starlark::runtime::starlark_fn_hasattr;
using ::starlark::runtime::starlark_fn_hash;
using ::starlark::runtime::starlark_fn_int;
using ::starlark::runtime::starlark_fn_len;
using ::starlark::runtime::starlark_fn_list;
using ::starlark::runtime::starlark_fn_ord;
using ::starlark::runtime::starlark_fn_print;
using ::starlark::runtime::starlark_fn_range;
using ::starlark::runtime::starlark_fn_repr;
using ::starlark::runtime::starlark_fn_reversed;
using ::starlark::runtime::starlark_fn_set;
using ::starlark::runtime::starlark_fn_str;
using ::starlark::runtime::starlark_fn_tuple;
using ::starlark::runtime::starlark_fn_type;
using ::starlark::runtime::starlark_fn_zip;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;
using ::starlark::runtime::starlark_types;

namespace starlark {
namespace interpreter {

namespace {

frame* create_frame(Arena& arena, const RepeatedPtrField<std::string>* names) {
  return Arena::Create<frame>(&arena, names);
}

class error_handler : public error_fn {
 public:
  error_handler(runner_state& state, logger& log, module_loader& loader) : state(state), log(log), loader(loader) {}

  void add_error(std::string_view error_msg) override {
    add_error(error_msg, starlark::logging::Position::default_instance());
  }

  void add_error(std::string_view error_msg, const starlark::logging::Position& pos) override {
    static Program fail_program = std::invoke([] -> Program {
      Program result;
      result.mutable_block()->Add()->add_op_code()->mutable_fail();
      return result;
    });
    static std::pair<Program*, std::string> base_program{&fail_program, "@@//:fail.star"};

    // Get the current operation that is being executed. Skip the operation if this is an internal module.
    std::pair<starlark::bytecode::Program*, std::string>* program_stack = nullptr;
    int block_ptr = 0;
    int instruction_ptr = 0;
     std::string_view source_code;
    if (state.instruction_ptr != 0) {
      auto current_module = loader.load_module(state.current_program.second);
      if (!current_module.ok()) {
        log.log(starlark::logging::LogLevel::LOG_LEVEL_ERROR, "internal error (0)", state.current_program.second, pos);
      } else if (!(*current_module)->inner()) {
        program_stack = &state.current_program;
        block_ptr = state.block_ptr;
        instruction_ptr = state.instruction_ptr;
        source_code = (*current_module)->source_code();
      }
    }
    for (int i = state.current_program_stack.size() - 1; program_stack == nullptr && i >= 0; --i) {
      auto current_module = loader.load_module(state.current_program_stack[i].second);
      if (!current_module.ok()) {
        log.log(starlark::logging::LogLevel::LOG_LEVEL_ERROR, "internal error (1)", state.current_program_stack[i].second, pos);
        break;
      }
      if (!(*current_module)->inner()) {
        program_stack = &state.current_program_stack[i];
        block_ptr = state.call_stack[i].block_ptr;
        instruction_ptr = state.call_stack[i].instruction_ptr;
        assert(!state.call_stack[i].inner);
        source_code = (*current_module)->source_code();
      }
    }

    // Log the error message.
    if (program_stack == nullptr) {
      log.log(starlark::logging::LogLevel::LOG_LEVEL_ERROR, error_msg, state.current_program.second, pos);
    } else {
      const auto& op_code = program_stack->first->block(block_ptr).op_code(instruction_ptr - 1);
      // If we have the position, then use it.
      if (op_code.has_highlight_start()) {
        auto msg = std::format("{}\n{}", error_msg, get_line_and_underline(source_code, op_code.highlight_start(), op_code.highlight_mid(), op_code.highlight_end()));
        log.log(starlark::logging::LogLevel::LOG_LEVEL_ERROR, msg, program_stack->second, op_code.highlight_mid());
      } else {
        // TODO(lmirelmann): Once all errors are converted to v2, this case should not exist.
        log.log(starlark::logging::LogLevel::LOG_LEVEL_ERROR, error_msg, program_stack->second, pos);
      }
    }

    // Push the current stack and trigger a fail.
    state.frame_stacks.push_back({});
    state.call_stack.push_back(call_stack_entry{.block_ptr = state.block_ptr, .instruction_ptr = state.instruction_ptr, .inner = state.inner});
    state.block_ptr = 0;
    state.instruction_ptr = 0;
    state.inner = true;
    state.current_program_stack.push_back(state.current_program);
    state.current_program = base_program;
  }

 private:
  runner_state& state;
  logger& log;
  module_loader& loader;
};

frame* run_program(
    module_loader& loader,
    std::pair<Program*, std::string>& starlark_program,
    bool inner,
    std::map<std::string, starlark_obj*, std::less<>>& global_context,
    context& ctx,
    logger& log) {
  std::vector<starlark_obj*> stack;
  std::vector<std::string_view> sv_stack;
  runner_state state;

  state.current_program = starlark_program;
  state.loader = &loader;
  state.inner = inner;
  ctx.runner_context(&state);

  error_handler error_callback(state, log, loader);
  frame* result = nullptr;

  // Limit checks.
  for (const auto& block : starlark_program.first->block()) {
    for (const auto& op : block.op_code()) {
      switch (op.op_code_case()) {
        case OpCode::kConstString:
          if (op.const_string().value().length() > ctx.options().max_string_length) {
            auto current_module = loader.load_module(starlark_program.second);
            if (!current_module.ok()) {
              error_callback.add_error("internal error (2)");
            } else {
              error_callback.add_error(error_v2_max_string_length(ctx.options().max_string_length, (*current_module)->source_code(), op.highlight_start(), op.highlight_end()), op.highlight_start());
            }
            return nullptr;
          }
          break;
        case OpCode::kConstBytes:
          if (op.const_bytes().value().length() > ctx.options().max_string_length) {
            auto current_module = loader.load_module(starlark_program.second);
            if (!current_module.ok()) {
              error_callback.add_error("internal error (3)");
            } else {
              error_callback.add_error(error_v2_max_bytes_length(ctx.options().max_string_length, (*current_module)->source_code(), op.highlight_start(), op.highlight_end()), op.highlight_start());
            }
            return nullptr;
          }
          break;
        case OpCode::kMakeTuple:
          if (op.make_tuple().number_of_elements() > ctx.options().max_sequence_size) {
            auto current_module = loader.load_module(starlark_program.second);
            if (!current_module.ok()) {
              error_callback.add_error("internal error (4)");
            } else {
              error_callback.add_error(error_v2_max_sequence_length(ctx.options().max_sequence_size, (*current_module)->source_code(), op.highlight_start(), op.highlight_end()), op.highlight_start());
            }
            return nullptr;
          }
          break;
        // TODO(lmirelmann): We should check that all ConstInt follow `log2_max_bigint`.
        default:
          break;
      }
    }
  }

  while (true) {
    const auto& op_code = state.current_program.first->block(state.block_ptr).op_code(state.instruction_ptr);
    state.instruction_ptr++;
    switch (op_code.op_code_case()) {
      case OpCode::kConstNone:
        stack.push_back(ctx.none_value());
        break;
      case OpCode::kConstInt:
        stack.push_back(Arena::Create<starlark_integer>(&ctx.arena(), op_code.const_int().value()));
        break;
      case OpCode::kConstBigInt:
        stack.push_back(Arena::Create<starlark_bigint>(&ctx.arena(),
            parse_number(op_code.const_big_int().value(), nullptr, 0)));
        break;
      case OpCode::kConstFloat:
        stack.push_back(Arena::Create<starlark_float>(&ctx.arena(), op_code.const_float().value()));
        break;
      case OpCode::kConstString:
        stack.push_back(Arena::Create<starlark_string>(&ctx.arena(), op_code.const_string().value()));
        break;
      case OpCode::kConstStringView:
        sv_stack.push_back(op_code.const_string_view().value());
        break;
      case OpCode::kConstBytes:
        stack.push_back(Arena::Create<starlark_bytes>(&ctx.arena(), op_code.const_bytes().value()));
        break;
      case OpCode::kMakeList:
        stack.push_back(Arena::Create<starlark_list>(&ctx.arena(), op_code.make_list().reserve_size()));
        break;
      case OpCode::kAddToList: {
        assert(stack.size() > op_code.add_to_list().number_of_elements());
        starlark_obj* candidate_list = stack[stack.size() - 1 - op_code.add_to_list().number_of_elements()];
        assert(candidate_list->type() == starlark_types::list_t);
        starlark_list* list = static_cast<starlark_list*>(candidate_list);
        assert(stack.size() >= op_code.add_to_list().number_of_elements());
        for (int i = 0; i < op_code.add_to_list().number_of_elements(); ++i) {
          list->append(stack[stack.size() - op_code.add_to_list().number_of_elements() + i], ctx, error_callback);
        }
        stack.resize(stack.size() - op_code.add_to_list().number_of_elements(), nullptr);
        break;
      }
      case OpCode::kMakeDictionary:
        // Note: The reserve size is not used.
        stack.push_back(Arena::Create<starlark_dictionary>(&ctx.arena()));
        break;
      case OpCode::kAddToDictionary: {
        assert(stack.size() > op_code.add_to_dictionary().number_of_elements() * 2);
        starlark_obj* candidate_dict = stack[stack.size() - 1 - op_code.add_to_dictionary().number_of_elements() * 2];
        assert(candidate_dict->type() == starlark_types::dict_t);
        starlark_dictionary* dict = static_cast<starlark_dictionary*>(candidate_dict);
        assert(op_code.add_to_dictionary().number_of_elements() < std::numeric_limits<decltype(op_code.add_to_list().number_of_elements())>::max() / 2);
        assert(stack.size() >= op_code.add_to_dictionary().number_of_elements() * 2);
        for (int i = 0; i < op_code.add_to_dictionary().number_of_elements(); ++i) {
          auto* key = stack[stack.size() - 2 * op_code.add_to_dictionary().number_of_elements() + 2 * i];
          if (!dict->insert(key, stack[stack.size() - 2 * op_code.add_to_dictionary().number_of_elements() + 2 * i + 1], error_callback).first &&
              op_code.add_to_dictionary().number_of_elements() > 1) {
            // This makes use of the fact that dictionary comprehensions always add elements one at a time and
            // that dictionary literals add all the elements in one go.
            error_callback.add_error(error_dictionary_duplicate_key(key->repr()));
            return nullptr;
          }
        }
        stack.resize(stack.size() - 2 * op_code.add_to_dictionary().number_of_elements(), nullptr);
        break;
      }
      case OpCode::kMakeTuple: {
        assert(stack.size() >= op_code.make_tuple().number_of_elements());
        starlark_tuple* result = Arena::Create<starlark_tuple>(&ctx.arena(), op_code.make_tuple().number_of_elements());
        for (int i = op_code.make_tuple().number_of_elements(); i > 0; --i) {
          result->add(stack[stack.size() - i]);
        }
        stack.resize(stack.size() - op_code.make_tuple().number_of_elements(), nullptr);
        stack.push_back(result);
        break;
      }
      case OpCode::kStore: {
        assert(!stack.empty());
        assert(state.frame_stacks.back().size() > op_code.store().frame());
        assert(state.frame_stacks.back()[state.frame_stacks.back().size() - 1 - op_code.store().frame()]->elements.size() > op_code.store().pos_in_frame());
        state.frame_stacks.back()[state.frame_stacks.back().size() - 1 - op_code.store().frame()]->elements[op_code.store().pos_in_frame()] = stack.back();
        stack.pop_back();
        break;
      }
      case OpCode::kLoad: {
        assert(state.frame_stacks.back().size() > op_code.load().frame());
        assert(state.frame_stacks.back()[state.frame_stacks.back().size() - 1 - op_code.load().frame()]->elements.size() > op_code.load().pos_in_frame());
        auto* value = state.frame_stacks.back()[state.frame_stacks.back().size() - 1 - op_code.load().frame()]->elements[op_code.load().pos_in_frame()];
        if (value == nullptr) {
          const auto& name = state.frame_stacks.back()[state.frame_stacks.back().size() - 1 - op_code.load().frame()]->names->Get(op_code.load().pos_in_frame());
          error_callback.add_error(error_unbound_variable(name));
          break;
        }
        stack.push_back(value);
        break;
      }
      case OpCode::kUnpack: {
        starlark_obj* element = stack.back();
        stack.pop_back();
        element->unpack(op_code.unpack().number_of_elements(), stack, ctx, error_callback);
        break;
      }
      case OpCode::kJumpIfFalse:
        if (!stack.back()->truthy()) {
          state.instruction_ptr += op_code.jump_if_false().address_delta() - 1;
        }
        stack.pop_back();
        break;
      case OpCode::kJumpIfTrueOrPop:
        if (stack.back()->truthy()) {
          state.instruction_ptr += op_code.jump_if_true_or_pop().address_delta() - 1;
        } else {
          stack.pop_back();
        }
        break;
      case OpCode::kJumpIfFalseOrPop:
        if (!stack.back()->truthy()) {
          state.instruction_ptr += op_code.jump_if_false_or_pop().address_delta() - 1;
        } else {
          stack.pop_back();
        }
        break;
      case OpCode::kCreateFrame:
        switch (op_code.create_frame().block_type()) {
          case BlockType::PREDECLARED_BLOCK: {
            assert(state.frame_stacks.empty());
            auto* global_frame = create_frame(ctx.arena(), &op_code.create_frame().symbol());
            int count = 0;
            for (const auto& symbol : op_code.create_frame().symbol()) {
              auto pos = global_context.find(symbol);
              if (pos == global_context.end()) {
                error_callback.add_error(error_symbol_not_available(symbol));
                return nullptr;
              } else {
                global_frame->elements[count] = pos->second;
                count++;
              }
            }
            state.frame_stacks.push_back({});
            state.frame_stacks.back().push_back(global_frame);
            break;
          }
          case BlockType::MODULE_BLOCK:
            assert(result == nullptr);
            result = create_frame(ctx.arena(), &op_code.create_frame().symbol());
            state.frame_stacks.back().push_back(result);
            break;
          case BlockType::FUNCTION_BLOCK:
            // This frame will be created by the function call.
            break;
          default:
            state.frame_stacks.back().push_back(create_frame(ctx.arena(), &op_code.create_frame().symbol()));
            break;
        }
        break;
      case OpCode::kPopFrame:
        state.frame_stacks.back().pop_back();
        break;
      case OpCode::kPop:
        stack.pop_back();
        break;
      case OpCode::kUnaryNot: {
        assert(!stack.empty());
        stack.back() = stack.back()->truthy() ? ctx.false_value() : ctx.true_value();
        break;
      }
      case OpCode::kUnaryPlus:
        assert(!stack.empty());
        stack.back() = stack.back()->unary_plus(ctx, error_callback);
        break;
      case OpCode::kUnaryMinus:
        stack.back() = stack.back()->unary_minus(ctx, error_callback);
        break;
      case OpCode::kUnaryTilde:
        stack.back() = stack.back()->unary_tilde(ctx, error_callback);
        break;
      case OpCode::kBinaryEqualsEquals: {
        assert(stack.size() >= 2);
        starlark_obj* element = stack.back();
        stack.pop_back();
        stack.back() = (stack.back()->equals(*element) ? ctx.true_value() : ctx.false_value());
        break;
      }
      case OpCode::kBinaryBangEquals: {
        assert(stack.size() >= 2);
        starlark_obj* element = stack.back();
        stack.pop_back();
        stack.back() = (stack.back()->equals(*element) ? ctx.false_value() : ctx.true_value());
        break;
      }
      case OpCode::kBinaryLessThan: {
        assert(stack.size() >= 2);
        starlark_obj* element = stack.back();
        stack.pop_back();
        auto cmp = stack.back()->cmp(*element, "<", error_callback);
        if (!cmp.ok()) {
          break;
        }
        stack.back() = (*cmp < 0 ? ctx.true_value() : ctx.false_value());
        break;
      }
      case OpCode::kBinaryLessThanEquals: {
        assert(stack.size() >= 2);
        starlark_obj* element = stack.back();
        stack.pop_back();
        auto cmp = stack.back()->cmp(*element, "<=", error_callback);
        if (!cmp.ok()) {
          break;
        }
        stack.back() = (*cmp <= 0 ? ctx.true_value() : ctx.false_value());
        break;
      }
      case OpCode::kBinaryGreaterThan: {
        assert(stack.size() >= 2);
        starlark_obj* element = stack.back();
        stack.pop_back();
        auto cmp = stack.back()->cmp(*element, ">", error_callback);
        if (!cmp.ok()) {
          break;
        }
        stack.back() = (*cmp > 0 ? ctx.true_value() : ctx.false_value());
        break;
      }
      case OpCode::kBinaryGreaterThanEquals: {
        assert(stack.size() >= 2);
        starlark_obj* element = stack.back();
        stack.pop_back();
        auto cmp = stack.back()->cmp(*element, ">=", error_callback);
        if (!cmp.ok()) {
          break;
        }
        stack.back() = (*cmp >= 0 ? ctx.true_value() : ctx.false_value());
        break;
      }
      case OpCode::kBinaryIn: {
        assert(stack.size() >= 2);
        starlark_obj* sequence = stack.back();
        stack.pop_back();
        stack.back() = sequence->binary_in(*stack.back(), error_callback) ? ctx.true_value() : ctx.false_value();
        break;
      }
      case OpCode::kBinaryNotIn: {
        assert(stack.size() >= 2);
        starlark_obj* sequence = stack.back();
        stack.pop_back();
        stack.back() = sequence->binary_in(*stack.back(), error_callback) ? ctx.false_value() : ctx.true_value();
        break;
      }
#define BINARY_OP(op, method)                                                              \
      case op: {                                                                           \
        assert(stack.size() >= 2);                                                         \
        starlark_obj* other = stack.back();                                                \
        stack.pop_back();                                                                  \
        stack.back() = stack.back()->method(*other, ctx, error_callback);                  \
        break;                                                                             \
      }
      BINARY_OP(OpCode::kBinaryLessThanLessThan, binary_lshift)
      BINARY_OP(OpCode::kBinaryGreaterThanGreaterThan, binary_rshift)
      BINARY_OP(OpCode::kBinaryPipe, binary_pipe)
      BINARY_OP(OpCode::kBinaryHat, binary_hat)
      BINARY_OP(OpCode::kBinaryAmpersand, binary_and)
      BINARY_OP(OpCode::kBinaryMinus, binary_minus)
      BINARY_OP(OpCode::kBinaryPlus, binary_plus)
      BINARY_OP(OpCode::kBinaryStar, binary_star)
      BINARY_OP(OpCode::kBinaryPercent, binary_percent)
      BINARY_OP(OpCode::kBinarySlash, binary_slash)
      BINARY_OP(OpCode::kBinarySlashSlash, binary_slash_slash)
#undef BINARY_OP
      case OpCode::kCall: {
        int args_count = op_code.call().positional_arguments_count() +
            op_code.call().named_arguments_count() +
            (op_code.call().has_variadic_positional_argument() ? 1 : 0) +
            (op_code.call().has_variadic_named_argument() ? 1 : 0);
        assert(stack.size() >= args_count + 1);
        assert(sv_stack.size() >= op_code.call().named_arguments_count());
        starlark_obj::pos_args_t pos_args;
        starlark_obj::named_args_t named_args;
        for (int i = 0; i < op_code.call().positional_arguments_count(); ++i) {
          pos_args.push_back(stack[stack.size() - args_count + i]);
        }
        for (int i = 0; i < op_code.call().named_arguments_count(); ++i) {
          auto* value = stack[stack.size() - args_count + op_code.call().positional_arguments_count() + i];
          named_args.insert(sv_stack[sv_stack.size() - op_code.call().named_arguments_count() + i], value);
        }
        if (op_code.call().has_variadic_positional_argument()) {
          auto* iterable = stack[stack.size() - args_count + op_code.call().positional_arguments_count() + op_code.call().named_arguments_count()];
          auto* it = iterable->get_iterator(true, ctx, error_callback);
          if (it == nullptr) {
            break;
          }
          while (it->has_next()) {
            pos_args.push_back(it->next());
          }
          it->end_iterator();
        }
        if (op_code.call().has_variadic_named_argument()) {
          auto* iterable = stack.back();
          // TODO(lmirelmann): This should be generalized if we want to support other types that are mappings.
          if (iterable->type() != starlark_types::dict_t) {
            error_callback.add_error(error_expect_mapping_after_star_star(iterable->type()));
            return nullptr;
          }
          auto* it = iterable->get_iterator(true, ctx, error_callback);
          if (it == nullptr) {
            // Should not happen.
            break;
          }
          while (it->has_next()) {
            auto* key = it->next();
            if (key->type() != starlark_types::string_t) {
              error_callback.add_error(error_v2_keyword_must_be_string());
              return nullptr;
            }
            auto* value = iterable->index(*key, ctx, error_callback);
            if (value == nullptr) {
              // Should not happen.
              return nullptr;
            }
            if (!named_args.insert(key->as_string(), value).second) {
              error_callback.add_error(error_multiple_values_for_keyword(key->as_string()));
              return nullptr;
            }
          }
          it->end_iterator();
        }
        stack.resize(stack.size() - args_count, nullptr);
        sv_stack.resize(sv_stack.size() - op_code.call().named_arguments_count(), std::string_view{});
        stack.back() = stack.back()->call(pos_args, named_args, ctx, error_callback);
        break;
      }
      case OpCode::kGetIterator:
        assert(!state.frame_stacks.back().empty());
        assert(!stack.empty());
        state.frame_stacks.back().back()->iterators.push_back(stack.back()->get_iterator(true, ctx, error_callback));
        stack.pop_back();
        break;
      case OpCode::kForIterator: {
        assert(!state.frame_stacks.back().empty());
        assert(!state.frame_stacks.back().back()->iterators.empty());
        auto* it = state.frame_stacks.back().back()->iterators.back();
        if (it->has_next()) {
          stack.push_back(it->next());
        } else {
          state.instruction_ptr += op_code.for_iterator().address_delta() - 1;
        }
        break;
      }
      case OpCode::kEndIterator:
        assert(!state.frame_stacks.back().empty());
        assert(!state.frame_stacks.back().back()->iterators.empty());
        state.frame_stacks.back().back()->iterators.back()->end_iterator();
        state.frame_stacks.back().back()->iterators.pop_back();
        break;
      case OpCode::kGoto:
        state.instruction_ptr += op_code.goto_().address_delta() - 1;
        break;
      case OpCode::kIndexMember: {
        assert(stack.size() >= 2);
        auto* index = stack.back();
        stack.pop_back();
        stack.back() = stack.back()->index(*index, ctx, error_callback);
        break;
      }
      case OpCode::kAssignIndexMember: {
        assert(stack.size() >= 3);
        auto* index = stack.back();
        stack.pop_back();
        auto* container = stack.back();
        stack.pop_back();
        auto* element = stack.back();
        stack.pop_back();
        container->index_assign(*index, *element, error_callback);
        break;
      }
      case OpCode::kDotMember:
        assert(stack.size() >= 1);
        stack.back() = stack.back()->dot(op_code.dot_member().member(), ctx, error_callback);
        break;
      case OpCode::kAssignDotMember: {
        assert(stack.size() >= 2);
        auto* element = stack.back();
        stack.pop_back();
        auto* value = stack.back();
        stack.pop_back();
        element->dot_assign(op_code.assign_dot_member().member(), *value, error_callback);
        break;
      }
#define COMPOUND_ASSIGN(op, op_method, method)                                                                                                                                 \
      case op: {                                                                                                                                                               \
        assert(stack.size() >= 1);                                                                                                                                             \
        assert(state.frame_stacks.back().size() > op_code.op_method().frame());                                                                                                \
        assert(state.frame_stacks.back()[state.frame_stacks.back().size() - 1 - op_code.op_method().frame()]->elements.size() > op_code.op_method().pos_in_frame());           \
        auto* value = state.frame_stacks.back()[state.frame_stacks.back().size() - 1 - op_code.op_method().frame()]->elements[op_code.op_method().pos_in_frame()];             \
        if (value == nullptr) {                                                                                                                                                \
          const auto& name = state.frame_stacks.back()[state.frame_stacks.back().size() - 1 - op_code.op_method().frame()]->names->Get(op_code.op_method().pos_in_frame());    \
          error_callback.add_error(error_unbound_variable(name));                                                                                                              \
          break;                                                                                                                                                               \
        }                                                                                                                                                                      \
        auto* element = stack.back();                                                                                                                                          \
        stack.pop_back();                                                                                                                                                      \
        auto* result = value->method(*element, ctx, error_callback);                                                                                                           \
        if (result == nullptr) {                                                                                                                                               \
          break;                                                                                                                                                               \
        }                                                                                                                                                                      \
        state.frame_stacks.back()[state.frame_stacks.back().size() - 1 - op_code.op_method().frame()]->elements[op_code.op_method().pos_in_frame()] = result;                  \
        break;                                                                                                                                                                 \
      }
      COMPOUND_ASSIGN(OpCode::kAssignPlusEquals, assign_plus_equals, plus_equals_assign)
      COMPOUND_ASSIGN(OpCode::kAssignMinusEquals, assign_minus_equals, minus_equals_assign)
      COMPOUND_ASSIGN(OpCode::kAssignStarEquals, assign_star_equals, star_equals_assign)
      COMPOUND_ASSIGN(OpCode::kAssignSlashEquals, assign_slash_equals, slash_equals_assign)
      COMPOUND_ASSIGN(OpCode::kAssignSlashSlashEquals, assign_slash_slash_equals, slash_slash_equals_assign)
      COMPOUND_ASSIGN(OpCode::kAssignPercentEquals, assign_percent_equals, percent_equals_assign)
      COMPOUND_ASSIGN(OpCode::kAssignAmpersandEquals, assign_ampersand_equals, ampersand_equals_assign)
      COMPOUND_ASSIGN(OpCode::kAssignPipeEquals, assign_pipe_equals, pipe_equals_assign)
      COMPOUND_ASSIGN(OpCode::kAssignHatEquals, assign_hat_equals, hat_equals_assign)
      COMPOUND_ASSIGN(OpCode::kAssignLessLessEquals, assign_less_less_equals, less_less_equals_assign)
      COMPOUND_ASSIGN(OpCode::kAssignGreaterGreaterEquals, assign_greater_greater_equals, greater_greater_equals_assign)
#undef COMPOUND_ASSIGN
#define INDEX_ASSIGN(op, method)                                                                                                                                         \
      case op: {                                                                                                                                                         \
        assert(stack.size() >= 3);                                                                                                                                       \
        auto* value = stack.back();                                                                                                                                      \
        stack.pop_back();                                                                                                                                                \
        auto* index = stack.back();                                                                                                                                      \
        stack.pop_back();                                                                                                                                                \
        auto* container = stack.back();                                                                                                                                  \
        stack.pop_back();                                                                                                                                                \
        auto* element = container->index(*index, ctx, error_callback);                                                                                                   \
        if (element == nullptr) {                                                                                                                                        \
          break;                                                                                                                                                         \
        }                                                                                                                                                                \
        auto* result = element->method(*value, ctx, error_callback);                                                                                                     \
        if (result == nullptr) {                                                                                                                                         \
          break;                                                                                                                                                         \
        }                                                                                                                                                                \
        container->index_assign(*index, *result, error_callback);                                                                                                        \
        break;                                                                                                                                                           \
      }
      INDEX_ASSIGN(OpCode::kAssignIndexMemberPlusEquals, plus_equals_assign)
      INDEX_ASSIGN(OpCode::kAssignIndexMemberMinusEquals, minus_equals_assign)
      INDEX_ASSIGN(OpCode::kAssignIndexMemberStarEquals, star_equals_assign)
      INDEX_ASSIGN(OpCode::kAssignIndexMemberSlashEquals, slash_equals_assign)
      INDEX_ASSIGN(OpCode::kAssignIndexMemberSlashSlashEquals, slash_slash_equals_assign)
      INDEX_ASSIGN(OpCode::kAssignIndexMemberPercentEquals, percent_equals_assign)
      INDEX_ASSIGN(OpCode::kAssignIndexMemberAmpersandEquals, ampersand_equals_assign)
      INDEX_ASSIGN(OpCode::kAssignIndexMemberPipeEquals, pipe_equals_assign)
      INDEX_ASSIGN(OpCode::kAssignIndexMemberHatEquals, hat_equals_assign)
      INDEX_ASSIGN(OpCode::kAssignIndexMemberLessLessEquals, less_less_equals_assign)
      INDEX_ASSIGN(OpCode::kAssignIndexMemberGreaterGreaterEquals, greater_greater_equals_assign)
#undef INDEX_ASSIGN
#define DOT_ASSIGN(op, op_method, method)                                                                                                                                \
      case op: {                                                                                                                                                         \
        assert(stack.size() >= 2);                                                                                                                                       \
        auto* value = stack.back();                                                                                                                                      \
        stack.pop_back();                                                                                                                                                \
        auto* element = stack.back();                                                                                                                                    \
        stack.pop_back();                                                                                                                                                \
        auto* field = element->dot(op_code.op_method().member(), ctx, error_callback);                                                                                   \
        if (field == nullptr) {                                                                                                                                          \
          break;                                                                                                                                                         \
        }                                                                                                                                                                \
        auto* result = field->method(*value, ctx, error_callback);                                                                                                       \
        if (result == nullptr) {                                                                                                                                         \
          break;                                                                                                                                                         \
        }                                                                                                                                                                \
        element->dot_assign(op_code.op_method().member(), *result, error_callback);                                                                                      \
        break;                                                                                                                                                           \
      }
      DOT_ASSIGN(OpCode::kAssignDotMemberPlusEquals, assign_dot_member_plus_equals, plus_equals_assign)
      DOT_ASSIGN(OpCode::kAssignDotMemberMinusEquals, assign_dot_member_minus_equals, minus_equals_assign)
      DOT_ASSIGN(OpCode::kAssignDotMemberStarEquals, assign_dot_member_star_equals, star_equals_assign)
      DOT_ASSIGN(OpCode::kAssignDotMemberSlashEquals, assign_dot_member_slash_equals, slash_equals_assign)
      DOT_ASSIGN(OpCode::kAssignDotMemberSlashSlashEquals, assign_dot_member_slash_slash_equals, slash_slash_equals_assign)
      DOT_ASSIGN(OpCode::kAssignDotMemberPercentEquals, assign_dot_member_percent_equals, percent_equals_assign)
      DOT_ASSIGN(OpCode::kAssignDotMemberAmpersandEquals, assign_dot_member_ampersand_equals, ampersand_equals_assign)
      DOT_ASSIGN(OpCode::kAssignDotMemberPipeEquals, assign_dot_member_pipe_equals, pipe_equals_assign)
      DOT_ASSIGN(OpCode::kAssignDotMemberHatEquals, assign_dot_member_hat_equals, hat_equals_assign)
      DOT_ASSIGN(OpCode::kAssignDotMemberLessLessEquals, assign_dot_member_less_less_equals, less_less_equals_assign)
      DOT_ASSIGN(OpCode::kAssignDotMemberGreaterGreaterEquals, assign_dot_member_greater_greater_equals, greater_greater_equals_assign)
#undef DOT_ASSIGN
      case OpCode::kAssignSliceRange: {
        assert(stack.size() >= 5);
        auto* stride = stack.back();
        stack.pop_back();
        auto* stop = stack.back();
        stack.pop_back();
        auto* start = stack.back();
        stack.pop_back();
        auto* container = stack.back();
        stack.pop_back();
        auto* element = stack.back();
        stack.pop_back();
        container->slice_range_assign(*start, *stop, *stride, *element, ctx, error_callback);
        break;
      }
#define ASSIGN_RANGE(op, method)                                                                                                                                         \
      case op: {                                                                                                                                                         \
        assert(stack.size() >= 5);                                                                                                                                       \
        auto* element = stack.back();                                                                                                                                    \
        stack.pop_back();                                                                                                                                                \
        auto* stride = stack.back();                                                                                                                                     \
        stack.pop_back();                                                                                                                                                \
        auto* stop = stack.back();                                                                                                                                       \
        stack.pop_back();                                                                                                                                                \
        auto* start = stack.back();                                                                                                                                      \
        stack.pop_back();                                                                                                                                                \
        auto* container = stack.back();                                                                                                                                  \
        stack.pop_back();                                                                                                                                                \
        container->method(*start, *stop, *stride, *element, ctx, error_callback);                                                                                        \
        break;                                                                                                                                                           \
      }
      ASSIGN_RANGE(OpCode::kAssignSliceRangePlusEquals, slice_range_plus_equals_assign)
      ASSIGN_RANGE(OpCode::kAssignSliceRangeMinusEquals, slice_range_minus_equals_assign)
      ASSIGN_RANGE(OpCode::kAssignSliceRangeStarEquals, slice_range_star_equals_assign)
      ASSIGN_RANGE(OpCode::kAssignSliceRangeSlashEquals, slice_range_slash_equals_assign)
      ASSIGN_RANGE(OpCode::kAssignSliceRangeSlashSlashEquals, slice_range_slash_slash_equals_assign)
      ASSIGN_RANGE(OpCode::kAssignSliceRangePercentEquals, slice_range_percent_equals_assign)
      ASSIGN_RANGE(OpCode::kAssignSliceRangeAmpersandEquals, slice_range_ampersand_equals_assign)
      ASSIGN_RANGE(OpCode::kAssignSliceRangePipeEquals, slice_range_pipe_equals_assign)
      ASSIGN_RANGE(OpCode::kAssignSliceRangeHatEquals, slice_range_hat_equals_assign)
      ASSIGN_RANGE(OpCode::kAssignSliceRangeLessLessEquals, slice_range_less_less_equals_assign)
      ASSIGN_RANGE(OpCode::kAssignSliceRangeGreaterGreaterEquals, slice_range_greater_greater_equals_assign)
#undef ASSIGN_RANGE
      case OpCode::kSliceRange: {
        assert(stack.size() >= 4);
        auto* stride = stack.back();
        stack.pop_back();
        auto* stop = stack.back();
        stack.pop_back();
        auto* start = stack.back();
        stack.pop_back();
        auto* element = stack.back();
        stack.back() = element->slice_range(*start, *stop, *stride, ctx, error_callback);
        break;
      }
      case OpCode::kMakeFunction: {
        assert(stack.size() >= op_code.make_function().default_values_count());
        std::vector<starlark_obj*> default_values;
        default_values.reserve(op_code.make_function().default_values_count());
        for (int i = op_code.make_function().default_values_count(); i > 0; --i) {
          default_values.push_back(stack[stack.size() - i]);
        }
        stack.resize(stack.size() - op_code.make_function().default_values_count(), nullptr);
        stack.push_back(Arena::Create<interpreter_function>(
            &ctx.arena(),
            op_code.make_function().entrypoint(),
            std::move(default_values),
            &state.current_program.first->block(op_code.make_function().entrypoint()).function_signature(),
            state.current_program.first,
            state.current_program.second,
            state.inner,
            &state.current_program.first->block(op_code.make_function().entrypoint()).op_code(0).create_frame().symbol(),
            state.frame_stacks.back()));
        break;
      }
      case OpCode::kReturn: {
        assert(state.frame_stacks.size() >= 2);
        assert(!state.frame_stacks.back().empty());
        assert(state.call_stack.size() >= 1);
        assert(stack.size() >= 2);
        state.frame_stacks.back().pop_back();
        state.frame_stacks.pop_back();
        state.block_ptr = state.call_stack.back().block_ptr;
        state.instruction_ptr = state.call_stack.back().instruction_ptr;
        state.inner = state.call_stack.back().inner;
        state.call_stack.pop_back();
        auto* result = stack.back();
        stack.pop_back();
        stack.back() = result;
        state.current_program = state.current_program_stack.back();
        state.current_program_stack.pop_back();
        if (--state.fns_in_stack[state.call_fns.back()] == 0) {
          state.fns_in_stack.erase(state.call_fns.back());
        }
        state.call_fns.pop_back();
        break;
      }
      case OpCode::kLoadModule: {
        auto mod_info = loader.load_module(op_code.load_module().module(), starlark_program.second);
        if (!mod_info.ok()) {
          error_callback.add_error(error_unable_to_load_module(op_code.load_module().module()));
          // It is quite hard to make this happen, but still possible.
          return nullptr;
        }
        if (!(*mod_info)->ready()) {
          error_callback.add_error(error_module_not_ready(op_code.load_module().module()));
          // It is quite hard to make this happen, but still possible.
          return nullptr;
        }
        auto* module_frame = (*mod_info)->get().first;
        assert(module_frame != nullptr);
        for (const auto& value : op_code.load_module().value()) {
          assert(state.frame_stacks.back().size() > value.pos().frame());
          assert(state.frame_stacks.back()[state.frame_stacks.back().size() - 1 - value.pos().frame()]->elements.size() > value.pos().pos_in_frame());
          bool found = false;
          for (int i = 0; i < module_frame->names->size(); ++i) {
            if (value.remote_symbol() == module_frame->names->Get(i)) {
              assert(module_frame->elements[i] != nullptr);
              state.frame_stacks.back()[state.frame_stacks.back().size() - 1 - value.pos().frame()]->elements[value.pos().pos_in_frame()] = module_frame->elements[i];
              found = true;
            }
          }
          if (!found) {
            error_callback.add_error(error_module_does_not_define_symbol(op_code.load_module().module(), value.remote_symbol()));
            return nullptr;
          }
        }
        break;
      }
      case OpCode::kEnd:
        assert(stack.empty());
        return result;
      case OpCode::kFail:
        return nullptr;
      case OpCode::OP_CODE_NOT_SET:
        error_callback.add_error(error_unknown_op(std::to_underlying(op_code.op_code_case())));
        return nullptr;
    }
  }
  return result;
}

std::string report_recursion_in_modules(const std::vector<std::string>& module_lookup, std::size_t initial_pos) {
  std::string result;

  for (std::size_t i = 0; i < initial_pos; ++i) {
    result += "    ";
    result += module_lookup[i];
    result += "\n";
  }
  result += "+-> ";
  result += module_lookup[initial_pos];
  result += "\n";
  for (std::size_t i = initial_pos + 1; i < module_lookup.size(); ++i) {
    result += "|   ";
    result += module_lookup[i];
    result += "\n";
  }
  result += "+-> ";
  result += module_lookup[initial_pos];
  result += "\n";
  return result;
}

}  // namespace

interpreter::interpreter() {}

status_or<frame*> interpreter::run(module_loader& loader,
                        std::string_view module_name,
                        const grammar_options& g_options,
                        const runtime_options& r_options,
                        logger& logging) {
  struct dependency {
    std::string module_name;
    std::string caller_module_name;
    Program* program;
  };
  std::vector<dependency> to_run;
  to_run.push_back(dependency{
    .module_name = std::string{module_name},
    .caller_module_name = "",
    .program = nullptr,
  });
  frame* last_frame = nullptr;
  std::vector<std::string> module_lookup;
  std::map<std::string, std::size_t, std::less<>> module_processing;
  bool module_reduction = false;
  while (!to_run.empty()) {
    // It would be nice not to have to retrieve the module every single time, but `module_info*` in
    // `module_loader` is not stable between runs.
    auto& entry = to_run.back();
    auto mod_info = loader.load_module(entry.module_name, entry.caller_module_name);
    if (!mod_info.ok()) {
      // TODO(lmirelmann): This should point to the part where the `load` statement is.
      logging.log(starlark::logging::LogLevel::LOG_LEVEL_ERROR,
                  std::format("ModuleNotFoundError: No module named '{}'", entry.module_name),
                  entry.caller_module_name,
                  starlark::logging::Position::default_instance());
      return status_or<frame*>(status_code::kStaticError);
    }
    if ((*mod_info)->ready()) {
      to_run.pop_back();
      module_reduction = true;
      continue;
    }

    // Begin - Find out whether there are recursions in modules.
    std::string_view c_name = (*mod_info)->cannonical_name();
    auto module_processing_it = module_processing.find(c_name);
    if (module_processing_it != module_processing.end() &&
        (!module_reduction || module_processing_it->second + 1 != module_lookup.size())) {
      logging.log(starlark::logging::LogLevel::LOG_LEVEL_ERROR,
                  std::format("recursion found during module lookup\n{}", report_recursion_in_modules(module_lookup, module_processing[std::string{c_name}])),
                  entry.caller_module_name,
                  starlark::logging::Position::default_instance());
      return status_or<frame*>(status_code::kStaticError);
    }
    if (module_processing_it == module_processing.end()) {
      module_processing[std::string{c_name}] = module_lookup.size();
      module_lookup.push_back(std::string{c_name});
    }
    module_reduction = false;
    // End - Find out whether there are recursions in modules.

    if (entry.program == nullptr) {
      std::set<std::string, std::less<>> binding;
      for (const auto& [key, value] : (*mod_info)->custom_binding()) {
        binding.insert(key);
      }
      class compiler star_compiler(binding);
      Program* starlark_program = star_compiler.compile((*mod_info)->cannonical_name(), (*mod_info)->source_code(), g_options, logging, (*mod_info)->arena());
      if (starlark_program == nullptr) {
        return status_or<frame*>(status_code::kStaticError);
      }
      entry.program = starlark_program;
      auto deps = get_dependencies(entry.program);
      bool needs_work = false;
      for (const auto& dep : deps) {
        to_run.push_back(dependency{
          .module_name = std::string{dep},
          .caller_module_name = std::string{(*mod_info)->cannonical_name()},
          .program = nullptr,
        });
        needs_work = true;
      }
      if (needs_work) {
        continue;
      }
    }
    context ctx((*mod_info)->arena(), r_options);

    std::map<std::string, starlark_obj*, std::less<>> global_context;
    add_base_global_context(global_context, ctx);
    for (const auto& kv : (*mod_info)->custom_binding()) {
      global_context.insert(kv);
    }
    auto current_program = std::make_pair(entry.program, std::string{(*mod_info)->cannonical_name()});
    last_frame = run_program(loader, current_program, (*mod_info)->inner(), global_context, ctx, logging);
    if (last_frame == nullptr) {
      return status_or<frame*>(status_code::kRuntimeError);
    }
    (*mod_info)->loaded(last_frame, entry.program);
    to_run.pop_back();
    module_processing.erase(std::string{c_name});
    module_lookup.pop_back();
    module_reduction = true;
  }
  return status_or<frame*>(last_frame);
}

void interpreter::add_base_global_context(std::map<std::string, starlark_obj*, std::less<>>& global_context, starlark::runtime::context& ctx) const {
  global_context["True"] = ctx.true_value();
  global_context["False"] = ctx.false_value();
  global_context["None"] = ctx.none_value();
  global_context[starlark_built_in_functions::abs_f] = create_function(ctx, nullptr, starlark_fn_abs, starlark_built_in_functions::abs_f);
  global_context[starlark_built_in_functions::all_f] = create_function(ctx, nullptr, starlark_fn_all, starlark_built_in_functions::all_f);
  global_context[starlark_built_in_functions::any_f] = create_function(ctx, nullptr, starlark_fn_any, starlark_built_in_functions::any_f);
  global_context[starlark_built_in_functions::bool_f] = create_function(ctx, nullptr, starlark_fn_bool, starlark_built_in_functions::bool_f);
  global_context[starlark_built_in_functions::bytes_f] = create_function(ctx, nullptr, starlark_fn_bytes, starlark_built_in_functions::bytes_f);
  global_context[starlark_built_in_functions::chr_f] = create_function(ctx, nullptr, starlark_fn_chr, starlark_built_in_functions::chr_f);
  global_context[starlark_built_in_functions::dict_f] = create_function(ctx, nullptr, starlark_fn_dict, starlark_built_in_functions::dict_f);
  global_context[starlark_built_in_functions::dir_f] = create_function(ctx, nullptr, starlark_fn_dir, starlark_built_in_functions::dir_f);
  global_context[starlark_built_in_functions::enumerate_f] = create_function(ctx, nullptr, starlark_fn_enumerate, starlark_built_in_functions::enumerate_f);
  global_context[starlark_built_in_functions::fail_f] = create_function(ctx, nullptr, starlark_fn_fail, starlark_built_in_functions::fail_f);
  global_context[starlark_built_in_functions::float_f] = create_function(ctx, nullptr, starlark_fn_float, starlark_built_in_functions::float_f);
  global_context[starlark_built_in_functions::getattr_f] = create_function(ctx, nullptr, starlark_fn_getattr, starlark_built_in_functions::getattr_f);
  global_context[starlark_built_in_functions::hasattr_f] = create_function(ctx, nullptr, starlark_fn_hasattr, starlark_built_in_functions::hasattr_f);
  global_context[starlark_built_in_functions::hash_f] = create_function(ctx, nullptr, starlark_fn_hash, starlark_built_in_functions::hash_f);
  global_context[starlark_built_in_functions::int_f] = create_function(ctx, nullptr, starlark_fn_int, starlark_built_in_functions::int_f);
  global_context[starlark_built_in_functions::len_f] = create_function(ctx, nullptr, starlark_fn_len, starlark_built_in_functions::len_f);
  global_context[starlark_built_in_functions::list_f] = create_function(ctx, nullptr, starlark_fn_list, starlark_built_in_functions::list_f);
  global_context[starlark_built_in_functions::max_f] = create_function(ctx, nullptr, starlark_fn_max_impl, starlark_built_in_functions::max_f);
  global_context[starlark_built_in_functions::min_f] = create_function(ctx, nullptr, starlark_fn_min_impl, starlark_built_in_functions::min_f);
  global_context[starlark_built_in_functions::ord_f] = create_function(ctx, nullptr, starlark_fn_ord, starlark_built_in_functions::ord_f);
  global_context[starlark_built_in_functions::print_f] = create_function(ctx, nullptr, starlark_fn_print, starlark_built_in_functions::print_f);
  global_context[starlark_built_in_functions::range_f] = create_function(ctx, nullptr, starlark_fn_range, starlark_built_in_functions::range_f);
  global_context[starlark_built_in_functions::repr_f] = create_function(ctx, nullptr, starlark_fn_repr, starlark_built_in_functions::repr_f);
  global_context[starlark_built_in_functions::reversed_f] = create_function(ctx, nullptr, starlark_fn_reversed, starlark_built_in_functions::reversed_f);
  global_context[starlark_built_in_functions::set_f] = create_function(ctx, nullptr, starlark_fn_set, starlark_built_in_functions::set_f);
  global_context[starlark_built_in_functions::sorted_f] = create_function(ctx, nullptr, starlark_fn_sorted_impl, starlark_built_in_functions::sorted_f);
  global_context[starlark_built_in_functions::str_f] = create_function(ctx, nullptr, starlark_fn_str, starlark_built_in_functions::str_f);
  global_context[starlark_built_in_functions::tuple_f] = create_function(ctx, nullptr, starlark_fn_tuple, starlark_built_in_functions::tuple_f);
  global_context[starlark_built_in_functions::type_f] = create_function(ctx, nullptr, starlark_fn_type, starlark_built_in_functions::type_f);
  global_context[starlark_built_in_functions::zip_f] = create_function(ctx, nullptr, starlark_fn_zip, starlark_built_in_functions::zip_f);
}

}  // namespace interpreter
}  // namespace starlark

