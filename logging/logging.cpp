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

logger::~logger() = default;

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

void logger::drop_last_error(LogLevel level) {
  if (!entries.empty() && entries.back().level() == level) {
    entries.pop_back();
  }
}

logger_wrap::logger_wrap(logger& delegate) : inner_logger(delegate),
  state(log_report{.debug = 0, .info = 0, .warning = 0, .error = 0, .fatal = 0}) {
}

void logger_wrap::log(LogLevel level, std::string_view message, std::string_view module, const starlark::logging::Position& pos) {
  switch (level) {
    case LogLevel::LOG_LEVEL_DEBUG:
      state.debug++;
      break;
    case LogLevel::LOG_LEVEL_INFO:
      state.info++;
      break;
    case LogLevel::LOG_LEVEL_WARNING:
      state.warning++;
      break;
    case LogLevel::LOG_LEVEL_ERROR:
      state.error++;
      break;
    case LogLevel::LOG_LEVEL_FATAL:
      state.fatal++;
      break;
    default:
      break;
  }
  inner_logger.log(level, message, module, pos);
}

void logger_wrap::drop_last_error(LogLevel level) {
  switch (level) {
    case LogLevel::LOG_LEVEL_DEBUG:
      if (state.debug) {
        state.debug--;
      }
      break;
    case LogLevel::LOG_LEVEL_INFO:
      if (state.info) {
        state.info--;
      }
      break;
    case LogLevel::LOG_LEVEL_WARNING:
      if (state.warning) {
        state.warning--;
      }
      break;
    case LogLevel::LOG_LEVEL_ERROR:
      if (state.error) {
        state.error--;
      }
      break;
    case LogLevel::LOG_LEVEL_FATAL:
      if (state.fatal) {
        state.fatal--;
      }
      break;
    default:
      break;
  }
  inner_logger.drop_last_error(level);
}

logger_wrap::log_report logger_wrap::report() {
  return state;
}

}  // namespace logging
}  // namespace starlark
