// Copyright 2026 Lucas Mirelmann

#include "runtime/parsing_options.hpp"

#include <charconv>
#include <string>

namespace starlark {
namespace runtime {

runtime_options get_runtime_options(std::string_view starlark_program, std::ostream& out) {
  runtime_options result = runtime_options{
    .out = out,
  };

#define NUM_PARAM(name)                                                                               \
  if (auto start = starlark_program.find("options." #name "("); start != std::string_view::npos) {    \
    start += std::strlen("options." #name "(");                                                       \
    auto end = starlark_program.find(")", start);                                                     \
    if (end == std::string_view::npos) {                                                              \
      std::cerr << "Unable to parse the runtime option `" #name "` (1)\n";                            \
    } else {                                                                                          \
      auto str = starlark_program.substr(start, end - start);                                         \
      int64_t value;                                                                                  \
      auto parse_result = std::from_chars(str.data(), str.data() + str.size(), value);                \
      if (parse_result.ec == std::errc::invalid_argument) {                                           \
        std::cerr << "Unable to parse the runtime option `" #name "` (2)\n";                          \
      } else {                                                                                        \
        result.name = value;                                                                          \
      }                                                                                               \
    }                                                                                                 \
  }

  NUM_PARAM(log2_max_bigint);
  NUM_PARAM(max_sequence_size);
  NUM_PARAM(max_string_length);
#undef NUM_PARAM

#define PARAM(name) \
  if (starlark_program.contains("options.no_" #name)) {       \
    result.name = false;                                      \
  } else if (starlark_program.contains("options." #name)) {   \
    result.name = true;                                       \
  }

  PARAM(allow_recursion);
#undef PARAM
  return result;
}

}  // namespace runtime
}  // namespace starlark

