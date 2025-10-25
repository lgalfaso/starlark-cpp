// Copyright 2024-2025 Lucas Mirelmann

#ifndef LOGGING_LOGGING_HPP_
#define LOGGING_LOGGING_HPP_

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

#include "grammar/token.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace logging {

// TODO(lmirelmann): Replace the logging entries with protobuf.
// TODO(lmirelmann): Move this class to another package that is not `grammar` specific.
// TODO(lmirelmann): Create a tee logger that allows us to receive a logger from an external source,
//   use it with a tee wrapper and still be able to know whether logs of a specific level were triggered
//   even if the logger was set to a level that this event would not be logged.
enum class log_level {
  kDebug,
  kInfo,
  kWarning,
  kError,
  kFatal,
  kNone,
};

struct log_entry {
  log_level level;
  std::string module;
  std::string message;
  grammar::position pos;
  std::chrono::time_point<std::chrono::system_clock> timestamp;
};

class logger {
 public:
  void set_level(log_level level);
  void log(log_level level, std::string_view message, std::string_view module, const starlark::grammar::position& pos);
  std::vector<starlark::logging::log_entry>::const_iterator begin() const;
  std::vector<starlark::logging::log_entry>::const_iterator end() const;
  bool empty() const;
  std::vector<starlark::logging::log_entry>::size_type size() const;

 private:
  std::vector<starlark::logging::log_entry> entries;
  log_level level = log_level::kWarning;
};


}  // namespace logging
}  // namespace starlark

#pragma GCC visibility pop

#endif  // LOGGING_LOGGING_HPP_

