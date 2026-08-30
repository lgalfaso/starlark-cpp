// Copyright 2026 Lucas Mirelmann

#ifndef PROTO_BYTECODE_FORMATTER_GOLDEN_UPDATER_HPP_
#define PROTO_BYTECODE_FORMATTER_GOLDEN_UPDATER_HPP_

#include <string>
#include <string_view>

namespace google {
namespace protobuf {
class Arena;
}  // namespace protobuf
}  // namespace google

namespace starlark {
namespace bytecode {
class Program;
}  // namespace bytecode

namespace proto {

bool compile_bytecode_from_star(std::string_view star_path,
    google::protobuf::Arena& arena,
    starlark::bytecode::Program*& program);

bool write_bytecode_txtpb_file(const starlark::bytecode::Program& program, std::string_view output_path);

int update_bytecode_goldens(std::string_view directory);

}  // namespace proto
}  // namespace starlark

#endif  // PROTO_BYTECODE_FORMATTER_GOLDEN_UPDATER_HPP_
