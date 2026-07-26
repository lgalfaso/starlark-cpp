// Copyright 2024-2026 Lucas Mirelmann

#include "logging/logging.hpp"

#include <string>
#include <vector>

namespace starlark {
namespace logging {

namespace {

LogEntry create_log(LogLevel level, std::string_view message, std::string_view module, const Position& pos) {
  LogEntry entry;
  entry.set_level(level);
  entry.set_module(module);
  entry.set_message(message);
  *entry.mutable_pos() = pos;
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

logger_wrap::log_report logger_wrap::report() {
  return state;
}

std::string pretty_log(const starlark::logging::LogEntry& entry) {
  std::string result;
  if (entry.module().empty()) {
    result += "<Unknown module>";
  } else {
    result += entry.module();
  }
  result += ":";
  if (entry.pos().has_row()) {
    result += std::to_string(entry.pos().row());
    result += ":";
    result += std::to_string(entry.pos().column());
    result += ":";
  }
  switch (entry.level()) {
    case LogLevel::LOG_LEVEL_DEBUG:
      result += " debug:";
      break;
    case LogLevel::LOG_LEVEL_INFO:
      result += " info:";
      break;
    case LogLevel::LOG_LEVEL_WARNING:
      result += " warning:";
      break;
    case LogLevel::LOG_LEVEL_ERROR:
      result += " error:";
      break;
    case LogLevel::LOG_LEVEL_FATAL:
      result += " fatal:";
      break;
    default:
      break;
  }
  result += " ";
  result += entry.message();
  return result;
}

}  // namespace logging
}  // namespace starlark
