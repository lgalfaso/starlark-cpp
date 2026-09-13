// Copyright 2024-2025 Lucas Mirelmann

#include "proto/base_proto_encoder.hpp"
#include "proto/starlark_bytecode.pb.h"

using ::starlark::bytecode::Program;

int main(int argc, char* argv[]) {
  return encode_proto<Program>(argc, argv);
}

