// Copyright 2026 Lucas Mirelmann

#ifndef RUNTIME_BUILTIN_POS_HPP_
#define RUNTIME_BUILTIN_POS_HPP_

#include "runtime/starlark_object.hpp"

namespace starlark {
namespace runtime {
namespace builtin_pos {

starlark_obj* abs_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback);
starlark_obj* all_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback);
starlark_obj* any_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback);
starlark_obj* bool_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback);
starlark_obj* bool_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback);
starlark_obj* bytes_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback);
starlark_obj* bytes_pos1(starlark_obj* this_obj, starlark_obj* source, context& ctx, error_fn& error_callback);
starlark_obj* chr_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback);
starlark_obj* dict_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback);
starlark_obj* dict_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback);
starlark_obj* dir_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback);
starlark_obj* enumerate_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback);
starlark_obj* enumerate_pos2(starlark_obj* this_obj, starlark_obj* iterable, starlark_obj* start, context& ctx, error_fn& error_callback);
starlark_obj* float_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback);
starlark_obj* float_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback);
starlark_obj* getattr_pos2(starlark_obj* this_obj, starlark_obj* element, starlark_obj* name, context& ctx, error_fn& error_callback);
starlark_obj* getattr_pos3(starlark_obj* this_obj, starlark_obj* element, starlark_obj* name, starlark_obj* default_value, context& ctx, error_fn& error_callback);
starlark_obj* hasattr_pos2(starlark_obj* this_obj, starlark_obj* element, starlark_obj* attr, context& ctx, error_fn& error_callback);
starlark_obj* hash_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback);
starlark_obj* int_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback);
starlark_obj* int_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback);
starlark_obj* int_pos2(starlark_obj* this_obj, starlark_obj* value, starlark_obj* base, context& ctx, error_fn& error_callback);
starlark_obj* len_pos1(starlark_obj* this_obj, starlark_obj* obj, context& ctx, error_fn& error_callback);
starlark_obj* list_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback);
starlark_obj* list_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback);
starlark_obj* max_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback);
starlark_obj* max_pos2(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, context& ctx, error_fn& error_callback);
starlark_obj* max_pos3(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, starlark_obj* a2, context& ctx, error_fn& error_callback);
starlark_obj* min_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback);
starlark_obj* min_pos2(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, context& ctx, error_fn& error_callback);
starlark_obj* min_pos3(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, starlark_obj* a2, context& ctx, error_fn& error_callback);
starlark_obj* ord_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback);
starlark_obj* print_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback);
starlark_obj* print_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback);
starlark_obj* range_pos1(starlark_obj* this_obj, starlark_obj* end, context& ctx, error_fn& error_callback);
starlark_obj* range_pos2(starlark_obj* this_obj, starlark_obj* start, starlark_obj* end, context& ctx, error_fn& error_callback);
starlark_obj* range_pos3(starlark_obj* this_obj, starlark_obj* start, starlark_obj* end, starlark_obj* step, context& ctx, error_fn& error_callback);
starlark_obj* repr_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback);
starlark_obj* reversed_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback);
starlark_obj* set_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback);
starlark_obj* set_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback);
starlark_obj* sorted_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback);
starlark_obj* str_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback);
starlark_obj* str_pos1(starlark_obj* this_obj, starlark_obj* object, context& ctx, error_fn& error_callback);
starlark_obj* tuple_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback);
starlark_obj* tuple_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback);
starlark_obj* type_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback);
starlark_obj* zip_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback);
starlark_obj* zip_pos1(starlark_obj* this_obj, starlark_obj* a0, context& ctx, error_fn& error_callback);
starlark_obj* zip_pos2(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, context& ctx, error_fn& error_callback);
starlark_obj* zip_pos3(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, starlark_obj* a2, context& ctx, error_fn& error_callback);

}  // namespace builtin_pos
}  // namespace runtime
}  // namespace starlark

#endif  // RUNTIME_BUILTIN_POS_HPP_
