// Copyright 2026 Lucas Mirelmann

#ifndef VM_FRAME_FACTORY_HPP_
#define VM_FRAME_FACTORY_HPP_

#include <string>

#include "google/protobuf/arena.h"
#include "google/protobuf/repeated_ptr_field.h"
#include "vm/frame.hpp"
#include "vm/module_metadata.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

inline frame* create_frame_on_arena(google::protobuf::Arena& arena,
    const google::protobuf::RepeatedPtrField<std::string>* names) {
  return google::protobuf::Arena::Create<frame>(&arena, names);
}

inline google::protobuf::RepeatedPtrField<std::string>* copy_frame_symbols_to_arena(google::protobuf::Arena& arena,
    const function_signature_metadata& fn_meta) {
  auto* symbols = google::protobuf::Arena::Create<google::protobuf::RepeatedPtrField<std::string>>(&arena);
  for (const auto& symbol : fn_meta.frame_symbols) {
    *symbols->Add() = symbol;
  }
  return symbols;
}

inline google::protobuf::RepeatedPtrField<std::string>* copy_frame_symbols_to_arena(google::protobuf::Arena& arena,
    const frame_metadata& metadata) {
  auto* symbols = google::protobuf::Arena::Create<google::protobuf::RepeatedPtrField<std::string>>(&arena);
  for (const auto& symbol : metadata.symbols) {
    *symbols->Add() = symbol;
  }
  return symbols;
}

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_FRAME_FACTORY_HPP_
