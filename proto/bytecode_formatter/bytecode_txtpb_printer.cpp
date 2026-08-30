// Copyright 2026 Lucas Mirelmann

#include "proto/bytecode_formatter/bytecode_txtpb_printer.hpp"

#include <algorithm>
#include <format>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "google/protobuf/text_format.h"
#include "proto/starlark_bytecode.pb.h"

namespace starlark {
namespace proto {
namespace {

using ::google::protobuf::Message;
using ::google::protobuf::TextFormat;

TextFormat::Printer make_printer() {
  TextFormat::Printer printer;
  printer.SetSingleLineMode(true);
  printer.SetRedactDebugString(false);
  return printer;
}

void trim_trailing_whitespace(std::string& text) {
  while (!text.empty() && (text.back() == '\n' || text.back() == '\r' || text.back() == ' ')) {
    text.pop_back();
  }
}

void print_message_inline(const Message& message, TextFormat::Printer& printer, std::ostream& out) {
  std::string text;
  if (!printer.PrintToString(message, &text)) {
    out << "{ }";
    return;
  }
  trim_trailing_whitespace(text);
  if (text.empty()) {
    out << "{ }";
    return;
  }
  out << "{ " << text << " }";
}

void print_block(const starlark::bytecode::Block& block, TextFormat::Printer& printer, std::ostream& out) {
  std::vector<std::string> op_lines;
  op_lines.reserve(static_cast<std::size_t>(block.op_code_size()));
  for (const auto& op_code : block.op_code()) {
    std::ostringstream line;
    line << "  op_code ";
    print_message_inline(op_code, printer, line);
    op_lines.push_back(line.str());
  }

  std::size_t max_width = 0;
  for (const auto& line : op_lines) {
    max_width = std::max(max_width, line.size());
  }

  const std::size_t max_index = op_lines.empty() ? 0 : op_lines.size() - 1;
  const int index_width = std::max(2, static_cast<int>(std::to_string(max_index).size()));

  out << "block {\n";
  for (std::size_t i = 0; i < op_lines.size(); ++i) {
    out << op_lines[i];
    if (op_lines[i].size() < max_width) {
      out << std::string(max_width - op_lines[i].size(), ' ');
    }
    out << std::format(" # {:>{}}\n", i, index_width);
  }
  if (block.has_function_signature()) {
    out << "  function_signature ";
    print_message_inline(block.function_signature(), printer, out);
    out << '\n';
  }
  out << "}\n";
}

}  // namespace

void print_bytecode_txtpb(const starlark::bytecode::Program& program, std::ostream& out) {
  TextFormat::Printer printer = make_printer();
  const auto* const_string_field =
      starlark::bytecode::Program::descriptor()->FindFieldByName("const_string");
  for (int i = 0; i < program.const_string_size(); ++i) {
    std::string value;
    TextFormat::PrintFieldValueToString(program, const_string_field, i, &value);
    trim_trailing_whitespace(value);
    out << "const_string: " << value << '\n';
  }
  for (const auto& block : program.block()) {
    print_block(block, printer, out);
  }
}

}  // namespace proto
}  // namespace starlark
