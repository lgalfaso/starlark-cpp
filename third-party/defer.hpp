// Lifted from https://mikejsavage.co.uk/cpp-tricks-defer/
// This code is released under the MIT license, which you can find at
//
// https://opensource.org/licenses/MIT

#ifndef THIRD_PARTY_DEFER_HPP_
#define THIRD_PARTY_DEFER_HPP_

#define CONCAT_HELPER(a, b) a##b
#define CONCAT(a, b) CONCAT_HELPER(a, b)
#define COUNTER_NAME(x) CONCAT(x, __COUNTER__)

template<typename F>
struct ScopeExit {
  explicit ScopeExit(F f_) : f(f_) {}
  ~ScopeExit() { f(); }
  F f;
};

struct DeferHelper {
  template<typename F>
  ScopeExit<F> operator+(F f) { return ScopeExit(f); }
};

#define defer [[maybe_unused]] const auto & COUNTER_NAME(DEFER_) = DeferHelper() + [&]()

#endif  // THIRD_PARTY_DEFER_HPP_

