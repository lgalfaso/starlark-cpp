// Copyright 2024-2026 Lucas Mirelmann

#include "interpreter/frame.hpp"

using ::google::protobuf::RepeatedPtrField;

namespace starlark {
namespace interpreter {

frame::frame(std::size_t size, const RepeatedPtrField<std::string>* names) : elements(size), names(names) {}

}  // namespace interpreter
}  // namespace starlark

