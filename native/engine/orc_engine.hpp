// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_ENGINE_ORC_ENGINE_HPP_
#define NATIVE_ENGINE_ORC_ENGINE_HPP_

#include <cstdint>

#include <filesystem>  // NOLINT(build/c++17)
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include "native/abi/starlark_module_abi.hpp"
#include "status_or/status.hpp"

#pragma GCC visibility push(default)

namespace llvm {
class LLVMContext;
class Module;
namespace orc {
class JITDylib;
class ThreadSafeModule;
}  // namespace orc
}  // namespace llvm

namespace starlark {
namespace native {

class orc_engine {
 public:
  orc_engine();
  ~orc_engine();

  orc_engine(const orc_engine&) = delete;
  orc_engine& operator=(const orc_engine&) = delete;

  starlark::result::status_or<const starlark_module_descriptor*> load_jit(uint64_t cache_key,
      std::unique_ptr<llvm::Module> module,
      std::unique_ptr<llvm::LLVMContext> context);

  starlark::result::status_or<const starlark_module_descriptor*> load_cached(uint64_t cache_key, std::filesystem::path dylib_path);

  const starlark_module_descriptor* find_loaded(uint64_t cache_key) const;

  starlark::result::status write_back(uint64_t cache_key, llvm::Module& module, std::filesystem::path dylib_path);

 private:
  starlark::result::status_or<const starlark_module_descriptor*> lookup_descriptor_symbol(llvm::orc::JITDylib& dylib,
      std::string_view symbol_name);

  struct impl;
  std::unique_ptr<impl> impl_;
  mutable std::mutex mutex_;
  std::map<uint64_t, const starlark_module_descriptor*> loaded_;
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_ENGINE_ORC_ENGINE_HPP_
