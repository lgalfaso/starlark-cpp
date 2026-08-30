// Copyright 2026 Lucas Mirelmann

#ifndef PROTO_BYTECODE_FORMATTER_TXTPB_PRINTER_HPP_
#define PROTO_BYTECODE_FORMATTER_TXTPB_PRINTER_HPP_

#include <ostream>

namespace starlark {
namespace bytecode {
class Program;
}  // namespace bytecode

namespace proto {

void print_bytecode_txtpb(const starlark::bytecode::Program& program, std::ostream& out);

}  // namespace proto
}  // namespace starlark

#endif  // PROTO_BYTECODE_FORMATTER_TXTPB_PRINTER_HPP_
