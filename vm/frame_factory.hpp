// Copyright 2026 Lucas Mirelmann

#ifndef VM_FRAME_FACTORY_HPP_
#define VM_FRAME_FACTORY_HPP_

#include <string>

#include "google/protobuf/repeated_ptr_field.h"
#include "runtime/starlark_object.hpp"
#include "vm/frame.hpp"
#include "vm/module_metadata.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

inline frame* create_frame_on_arena(starlark::runtime::context& ctx,
    const google::protobuf::RepeatedPtrField<std::string>* names) {
  return ctx.create<frame>(names);
}

inline google::protobuf::RepeatedPtrField<std::string>* copy_frame_symbols_to_arena(starlark::runtime::context& ctx,
    const function_signature_metadata& fn_meta) {
  auto* symbols = ctx.create<google::protobuf::RepeatedPtrField<std::string>>();
  for (const auto& symbol : fn_meta.frame_symbols) {
    *symbols->Add() = symbol;
  }
  return symbols;
}

inline google::protobuf::RepeatedPtrField<std::string>* copy_frame_symbols_to_arena(starlark::runtime::context& ctx,
    const frame_metadata& metadata) {
  auto* symbols = ctx.create<google::protobuf::RepeatedPtrField<std::string>>();
  for (const auto& symbol : metadata.symbols) {
    *symbols->Add() = symbol;
  }
  return symbols;
}

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_FRAME_FACTORY_HPP_
