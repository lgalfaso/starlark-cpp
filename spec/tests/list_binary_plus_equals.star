def run():
  a = [1, 2]
  a += [3]
  assert_eq(a, [1, 2, 3])
  a += a
  assert_eq(a, [1, 2, 3, 1, 2, 3])

run()

assert_fail('''
def foo():
  a = []
  a += ()

foo()
''')

assert_fail('''
def foo():
  a = [1]
  for x in a:
    a += [x]

foo()
''')


# TEST(StarlarkList, PlusEqualsAssignNotList) {
#   starlark_list list(0);
#   starlark_tuple tuple(0);
#   Arena arena;
#   context ctx(arena);
#   error_handler error_callback;
# 
#   auto* result = list.plus_equals_assign(tuple, ctx, error_callback);
#   EXPECT_EQ(result, nullptr);
#   ASSERT_THAT(error_callback.messages, SizeIs(1));
#   EXPECT_EQ(error_callback.messages[0], "TypeError: can only concatenate list (not \"tuple\") to list");
# }
# 
# TEST(StarlarkList, PlusEqualsAssignWhileIterating) {
#   starlark_list list(0);
#   starlark_integer zero(0);
#   starlark_integer one(1);
#   Arena arena;
#   context ctx(arena);
#   error_handler error_callback;
#   list.append(&zero, ctx, error_callback);
#   list.append(&one, ctx, error_callback);
# 
#   [[maybe_unused]] auto* it = list.get_iterator(true, ctx, error_callback);
#   EXPECT_THAT(error_callback.messages, IsEmpty());
#   list.plus_equals_assign(list, ctx, error_callback);
#   ASSERT_THAT(error_callback.messages, SizeIs(1));
#   EXPECT_EQ("Error in append: list value is temporarily immutable due to active for-loop iteration", error_callback.messages[0]);
# }
# 
