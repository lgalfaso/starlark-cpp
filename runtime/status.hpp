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
  explicit status_or(status_code code);
  explicit status_or(const T& value);
  explicit status_or(T&& value);
  bool ok() const;

  const T& operator*() const;
  T& operator*();
  const T* operator->() const;
  T* operator->();

 private:
  status_code code;
  T value;
};

template<typename T>
status_or<T>::status_or(status_code code) : code(code) {}

template<typename T>
status_or<T>::status_or(const T& value) : code(status_code::kOk), value(value) {}

template<typename T>
status_or<T>::status_or(T&& value) : code(status_code::kOk), value(std::forward<T>(value)) {}

template<typename T>
bool status_or<T>::ok() const {
  return code == status_code::kOk;
}
  
template<typename T>
const T& status_or<T>::operator*() const {
  return value;
}

template<typename T>
T& status_or<T>::operator*() {
  return value;
}

template<typename T>
const T* status_or<T>::operator->() const {
  return &value;
}

template<typename T>
T* status_or<T>::operator->() {
  return &value;
}

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STATUS_HPP_

