// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_ABI_STARLARK_MODULE_ABI_HPP_
#define NATIVE_ABI_STARLARK_MODULE_ABI_HPP_

#include <cstdint>

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {
class context;
class error_fn;
class starlark_obj;
}  // namespace runtime

namespace native {

static constexpr uint32_t kNativeAbiVersion = 43;

// Bump cache keys when the JIT LLVM version changes so stale dylibs are not reused.
static constexpr const char* kNativeJitLlvmVersion = "17.0.4.bcr.1";

struct module_runtime_state;
struct native_exec_context;

typedef starlark::runtime::starlark_obj* (*native_fn_t)(native_exec_context* exec,
    starlark::runtime::starlark_obj** argv,
    int32_t argc,
    starlark::runtime::context* ctx,
    starlark::runtime::error_fn* err);

struct starlark_module_descriptor {
  bool (*init)(module_runtime_state* self, starlark::runtime::error_fn& err);
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_ABI_STARLARK_MODULE_ABI_HPP_
