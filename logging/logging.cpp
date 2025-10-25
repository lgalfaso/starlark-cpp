// Copyright 2024-2025 Lucas Mirelmann

#include "logging/logging.hpp"

#include <chrono>
#include <string>
#include <vector>

namespace starlark {
namespace logging {

namespace {

LogEntry create_log(LogLevel level, std::string_view message, std::string_view module, const Position& pos) {
  const std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now();
  auto now_in_seconds = std::chrono::duration_cast<std::chrono::seconds>(
      now.time_since_epoch()).count();
  auto now_in_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
      now.time_since_epoch()).count();
  LogEntry entry;
  entry.set_level(level);
  entry.set_module(module);
  entry.set_message(message);
  *entry.mutable_pos() = pos;
  entry.mutable_timestamp()->set_seconds(now_in_seconds);
  entry.mutable_timestamp()->set_nanos(now_in_ns % 1'000'000'000);
  return entry;
}

}  // namespace

void logger::set_level(LogLevel level) {
  this->level = level;
}

void logger::log(LogLevel level, std::string_view message, std::string_view module, const Position& pos) {
  if (level < this->level) {
    return;
  }
  entries.emplace_back(create_log(level, message, module, pos));
}

std::vector<LogEntry>::const_iterator logger::begin() const {
  return entries.begin();
}

std::vector<LogEntry>::const_iterator logger::end() const {
  return entries.end();
}

bool logger::empty() const {
  return entries.empty();
}

std::vector<LogEntry>::size_type logger::size() const {
  return entries.size();
}

}  // namespace logging
}  // namespace starlark
