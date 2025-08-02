// Copyright 2024-2025 Lucas Mirelmann

#include "proto/starlark_bytecode.pb.h"
#include "proto/base_proto_encoder.hpp"

using starlark::bytecode::Program;

int main(int argc, char* argv[]) {
  return encode_proto<Program>(argc, argv);
}

