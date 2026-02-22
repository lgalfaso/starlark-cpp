// Copyright 2024-2026 Lucas Mirelmann

#include "interpreter/frame.hpp"

#include <string>

using ::google::protobuf::RepeatedPtrField;

namespace starlark {
namespace interpreter {

frame::frame(const RepeatedPtrField<std::string>* names) : elements(names->size()), names(names) {}

}  // namespace interpreter
}  // namespace starlark

