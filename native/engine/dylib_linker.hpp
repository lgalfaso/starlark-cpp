// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_ENGINE_DYLIB_LINKER_HPP_
#define NATIVE_ENGINE_DYLIB_LINKER_HPP_

#include <filesystem>
#include <string>

#include "llvm/TargetParser/Triple.h"

namespace starlark {
namespace native {

bool link_object_to_dylib(const llvm::Triple& triple,
    const std::filesystem::path& object_path,
    const std::filesystem::path& dylib_path);

}  // namespace native
}  // namespace starlark

#endif  // NATIVE_ENGINE_DYLIB_LINKER_HPP_
