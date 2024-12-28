// Copyright 2024 Lucas Mirelmann

#include "grammar/logging.hpp"

namespace grammar {

log_entry log(log_level level, std::string_view message, std::string_view module, const grammar::position& pos) {
  return log_entry{
    .level = level,
    .module = std::string{module},
    .message = std::string{message},
    .pos = pos,
    .timestamp = std::chrono::system_clock::now(),
  };
}

}  // namespace grammar
