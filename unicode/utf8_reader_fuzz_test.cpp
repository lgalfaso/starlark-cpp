// Copyright 2024-2025 Lucas Mirelmann

#include <string_view>

#include "unicode/utf8_reader.hpp"

using starlark::unicode::utf8_reader;

namespace {

void Utf8ReaderFuzzing(const char* data, size_t size) {
  utf8_reader reader(std::string_view(data, size), true, true);
  while (reader.pending()) {
    reader.peek_code_point();
    reader.skip_code_point();
  }
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  Utf8ReaderFuzzing(reinterpret_cast<const char*>(data), size);
  return 0;
}

