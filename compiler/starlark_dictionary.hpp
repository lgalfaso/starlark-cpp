// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_DICTIONARY_HPP_
#define COMPILER_STARLARK_DICTIONARY_HPP_

#include <string>

#include "containers/linked_hash_map.hpp"
#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_dictionary : public starlark_obj {
 public:
  const std::string& type() const override;
  bool truthy() const override;
  bool equals(const starlark_obj& other) const override;
  int64_t hash() const override;
  starlark_dictionary& insert(starlark_obj* key, starlark_obj* value);

 protected:
  bool inner_repr(printer& print, uint64_t pos) const override;

 private:
  static const std::string type_value;
  starlark::cnt::linked_hash_map<starlark_obj*, starlark_obj*, starlark_hash, starlark_equals_to> values;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_DICTIONARY_HPP_

