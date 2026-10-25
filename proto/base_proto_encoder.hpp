// Copyright 2024-2025 Lucas Mirelmann

#ifndef PROTO_BASE_PROTO_ENCODER_HPP_
#define PROTO_BASE_PROTO_ENCODER_HPP_

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

#include <filesystem>  // NOLINT(build/c++17)
#include <fstream>

using google::protobuf::TextFormat;
using google::protobuf::io::IstreamInputStream;

template<typename T>
int encode_proto(int argc, char* argv[]) {
  if (argc != 3) {
    return 1;
  }

  std::ifstream in(argv[1]);
  if (!in) {
    return 2;
  }
  T starlark_type;
  IstreamInputStream input(&in);
  if (!TextFormat::Parse(&input, &starlark_type)) {
    return 3;
  }

  if (std::filesystem::exists(argv[2])) {
    return 4;
  }
  std::ofstream out(argv[2], std::ios::binary);
  if (!out) {
    return 4;
  }
  if (!starlark_type.SerializeToOstream(&out)) {
    return 5;
  }

  return 0;
}

#endif  // PROTO_BASE_PROTO_ENCODER_HPP_
