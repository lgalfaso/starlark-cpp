// Copyright 2024-2025 Lucas Mirelmann

#ifndef PROTO_BASE_PROTO_ENCODER_HPP_
#define PROTO_BASE_PROTO_ENCODER_HPP_

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <memory>

#include "third-party/defer.hpp"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

using google::protobuf::TextFormat;
using google::protobuf::io::FileInputStream;

template<typename T>
int encode_proto(int argc, char* argv[]) {
  if (argc != 3) {
    return 1;
  }

  int in_fd = open(argv[1], O_RDONLY);
  if (in_fd < 0) {
    return 2;
  }
  defer { close(in_fd); };

  auto input = std::make_unique<FileInputStream>(in_fd);
  T starlark_type;
  if (!TextFormat::Parse(input.get(), &starlark_type)) {
    return 3;
  }

#ifdef __APPLE__
  int out_fd = open(argv[2], O_WRONLY | O_CREAT | O_EXCL);
#else
  int out_fd = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, S_IRUSR);
#endif
  if (out_fd < 0) {
    return 4;
  }
  defer { close(out_fd); };

  if (!starlark_type.SerializeToFileDescriptor(out_fd)) {
    return 5;
  }
  if (fchmod(out_fd, S_IRUSR) != 0) {
    return 6;
  }

  return 0;
}

#endif  // PROTO_BASE_PROTO_ENCODER_HPP_

