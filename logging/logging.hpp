// Copyright 2024-2025 Lucas Mirelmann

#ifndef LOGGING_LOGGING_HPP_
#define LOGGING_LOGGING_HPP_

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

#include "proto/starlark_logging.pb.h"

#pragma GCC visibility push(default)

namespace starlark {
namespace logging {

// TODO(lmirelmann): Create a tee logger that allows us to receive a logger from an external source,
//   use it with a tee wrapper and still be able to know whether logs of a specific level were triggered
//   even if the logger was set to a level that this event would not be logged.

class logger {
 public:
  virtual ~logger();
  void set_level(starlark::logging::LogLevel level);
  virtual void log(starlark::logging::LogLevel level, std::string_view message, std::string_view module, const starlark::logging::Position& pos);
  std::vector<starlark::logging::LogEntry>::const_iterator begin() const;
  std::vector<starlark::logging::LogEntry>::const_iterator end() const;
  bool empty() const;
  std::vector<starlark::logging::LogEntry>::size_type size() const;

 private:
  std::vector<starlark::logging::LogEntry> entries;
  starlark::logging::LogLevel level = starlark::logging::LogLevel::LOG_LEVEL_WARNING;
};

class logger_wrap : public logger {
 public:
  struct log_report {
    int debug;
    int info;
    int warning;
    int error;
    int fatal;
  };

  logger_wrap(logger&);
  void log(starlark::logging::LogLevel level, std::string_view message, std::string_view module, const starlark::logging::Position& pos) override;
  log_report report();

 private:
  logger& inner_logger;
  log_report state;
};

}  // namespace logging
}  // namespace starlark

#pragma GCC visibility pop

#endif  // LOGGING_LOGGING_HPP_

