// Copyright 2026 Lucas Mirelmann

#include "native/engine/cache.hpp"

#include <charconv>
#include <cstring>
#include <functional>
#include <string_view>

#include "native/abi/starlark_module_abi.hpp"
#include "runtime/hex_format.hpp"

namespace starlark {
namespace native {

namespace {

uint64_t fnv1a64(std::string_view data) {
  uint64_t hash = 14695981039346656037ULL;
  for (unsigned char c : data) {
    hash ^= c;
    hash *= 1099511628211ULL;
  }
  return hash;
}

uint64_t mix(uint64_t a, uint64_t b) {
  a ^= b + 0x9e3779b97f4a7c15ULL + (a << 6) + (a >> 2);
  return a;
}

}  // namespace

uint64_t compute_cache_key(const cache_key_parts& parts) {
  uint64_t hash = fnv1a64(parts.module_source);
  hash = mix(hash, fnv1a64(parts.target_triple));
  hash = mix(hash, static_cast<uint64_t>(kNativeAbiVersion));
  hash = mix(hash, fnv1a64(kNativeJitLlvmVersion));
  for (uint64_t dep : parts.dep_keys) {
    hash = mix(hash, dep);
  }
  return hash;
}

std::filesystem::path default_cache_root() {
  if (const char* env = std::getenv("STARLARK_NATIVE_CACHE")) {
    return std::filesystem::path{env};
  }
  if (const char* home = std::getenv("HOME")) {
    return std::filesystem::path {home} / ".cache" / "starlark-native";
  }
  return std::filesystem::path{"/tmp/starlark-native-cache"};
}

std::filesystem::path dylib_path_for_key(uint64_t cache_key, std::filesystem::path cache_root) {
  // {cache_root}/{kNativeAbiVersion}/{key[0:2]}/{key[2:16]}/module.dylib
  static constexpr char kSuffix[] = "/module.dylib";

  char key_hex[16];
  starlark::runtime::write_hex16_lower(cache_key, key_hex);

  char rel[64];
  char* p = rel;
  p = std::to_chars(p, rel + sizeof(rel), kNativeAbiVersion).ptr;
  *p++ = '/';
  std::memcpy(p, key_hex, 2);
  p += 2;
  *p++ = '/';
  std::memcpy(p, key_hex + 2, 14);
  p += 14;
  std::memcpy(p, kSuffix, sizeof(kSuffix) - 1);
  p += sizeof(kSuffix) - 1;

  return cache_root / std::filesystem::path(std::string_view(rel, static_cast<std::size_t>(p - rel)));
}

std::optional<std::filesystem::path> find_cached_dylib(uint64_t cache_key, std::filesystem::path cache_root) {
  auto path = dylib_path_for_key(cache_key, cache_root);
  if (std::filesystem::exists(path)) {
    return path;
  }
  return std::nullopt;
}

}  // namespace native
}  // namespace starlark
