// Copyright 2024-2025 Lucas Mirelmann

#include "grammar/logging.hpp"

#include <string>
#include <vector>

namespace grammar {

namespace {

log_entry create_log(log_level level, std::string_view message, std::string_view module, const grammar::position& pos) {
  return log_entry{
    .level = level,
    .module = std::string{module},
    .message = std::string{message},
    .pos = pos,
    .timestamp = std::chrono::system_clock::now(),
  };
}

}  // namespace

void logger::set_level(log_level level) {
  this->level = level;
}

void logger::log(log_level level, std::string_view message, std::string_view module, const grammar::position& pos) {
  if (level < this->level) {
    return;
  }
  entries.emplace_back(create_log(level, message, module, pos));
}

std::vector<grammar::log_entry>::const_iterator logger::begin() const {
  return entries.begin();
}

std::vector<grammar::log_entry>::const_iterator logger::end() const {
  return entries.end();
}

bool logger::empty() const {
  return entries.empty();
}

std::vector<grammar::log_entry>::size_type logger::size() const {
  return entries.size();
}

}  // namespace grammar
