// Copyright 2024 Lucas Mirelmann

#ifndef GRAMMAR_LOGGING_HPP_
#define GRAMMAR_LOGGING_HPP_

#include <chrono>

#include "grammar/token.hpp"

#pragma GCC visibility push(default)

namespace grammar {

// TODO(lmirelmann): This is very basic, there should be a class that is used to handle all logging.
enum class log_level {
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  FATAL,
};

struct log_entry {
  log_level level;
  std::string module;
  std::string message;
  grammar::position pos;
  std::chrono::time_point<std::chrono::system_clock> timestamp;
};

log_entry log(log_level level, std::string_view message, std::string_view module, const grammar::position& pos);

}  // namespace grammar

#pragma GCC visibility pop

#endif  // GRAMMAR_LOGGING_HPP_

