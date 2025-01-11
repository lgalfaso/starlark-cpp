// Copyright 2024 Lucas Mirelmann

#include <string_view>

#include "unicode/utf8_reader.hpp"

using unicode::utf8_reader;

void Utf8ReaderFuzzing(char* data, size_t size) {
  utf8_reader reader(std::string_view(data, size), true);
  while (reader.pending()) {
    reader.peek_code_point();
    reader.skip_code_point();
  }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  Utf8ReaderFuzzing((char*)data, size);
  return 0;
}

