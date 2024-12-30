// Copyright 2024 Lucas Mirelmann

#ifndef GRAMMAR_LOGGING_HPP_
#define GRAMMAR_LOGGING_HPP_

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

#include "grammar/token.hpp"

#pragma GCC visibility push(default)

namespace grammar {

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

class logger {
 public:
  void set_level(log_level level);
  void log(log_level level, std::string_view message, std::string_view module, const grammar::position& pos);
  std::vector<grammar::log_entry>::const_iterator begin() const;
  std::vector<grammar::log_entry>::const_iterator end() const;
  bool empty() const;
  std::vector<grammar::log_entry>::size_type size() const;

 private:
  std::vector<grammar::log_entry> entries;
  log_level level = log_level::WARNING;
};


}  // namespace grammar

#pragma GCC visibility pop

#endif  // GRAMMAR_LOGGING_HPP_

