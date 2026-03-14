// Copyright 2026 Lucas Mirelmann

#ifndef RUNTIME_STATUS_HPP_
#define RUNTIME_STATUS_HPP_

#include <utility>

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

enum class status_code {
  kOk,
  kError,
};

class status {
 public:
  explicit status(status_code code);
  bool ok() const;

 private:
  status_code code;
};

status ok_status();
status error_status();

template<typename T>
class status_or {
 public:
  explicit status_or(status_code code) : code(code) {}
  explicit status_or(const T& value) : code(status_code::kOk), value(value) {}
  explicit status_or(T&& value) : code(status_code::kOk), value(std::forward<T>(value)) {}

  bool ok() const {
    return code == status_code::kOk;
  }

  const T& operator*() const {
    return value;
  }

  T& operator*() {
    return value;
  }

  const T* operator->() const {
    return &value;
  }

  T* operator->() {
    return &value;
  }

 private:
  status_code code;
  T value;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STATUS_HPP_

