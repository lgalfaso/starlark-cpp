// Copyright 2024-2026 Lucas Mirelmann

#include "vm/frame.hpp"

#include <string>

using ::google::protobuf::RepeatedPtrField;

namespace starlark {
namespace vm {

frame::frame(const RepeatedPtrField<std::string>* names) : elements(names->size()), names(names) {}

}  // namespace vm
}  // namespace starlark
