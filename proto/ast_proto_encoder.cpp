// Copyright 2024-2025 Lucas Mirelmann

#include "proto/base_proto_encoder.hpp"
#include "proto/starlark_ast.pb.h"

using ::starlark::ast::File;

int main(int argc, char* argv[]) {
  return encode_proto<File>(argc, argv);
}

