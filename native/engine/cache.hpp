// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_CACHE_HPP_
#define NATIVE_CACHE_HPP_

#include <cstdint>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

struct cache_key_parts {
  std::string_view module_source;
  std::vector<uint64_t> dep_keys;
  std::string_view target_triple;
};

uint64_t compute_cache_key(const cache_key_parts& parts);

std::filesystem::path default_cache_root();

std::optional<std::filesystem::path> find_cached_dylib(uint64_t cache_key, std::filesystem::path cache_root);

std::filesystem::path dylib_path_for_key(uint64_t cache_key, std::filesystem::path cache_root);

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_CACHE_HPP_
