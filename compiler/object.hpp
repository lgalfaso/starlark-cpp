#ifndef COMPILER_OBJECT_HPP_
#define COMPILER_OBJECT_HPP_

#include <string>

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_obj {
 public:
  virtual ~starlark_obj();
  virtual const std::string& type() const = 0;
  virtual std::string str() const;
  virtual std::string repr() const = 0;
  virtual bool truthy() const = 0;
  virtual bool equals(const starlark_obj& other) const = 0;
  virtual int64_t hash() const = 0;
};

struct starlark_hash {
  size_t operator()(const starlark_obj* value) const;
};

struct starlark_equals_to {
  bool operator()(const starlark_obj* lhs, const starlark_obj* rhs) const;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_OBJECT_HPP_

