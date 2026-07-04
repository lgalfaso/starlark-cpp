# No arguments.
assert_fail('''b'abc'.partition()''')
assert_fail('''b'abc'.rpartition()''')

# One argument.
assert_fail('''b'abc'.partition(None)''')
assert_fail('''b'abc'.rpartition(None)''')
assert_fail('''b'abc'.partition(97)''')
assert_fail('''b'abc'.rpartition(97)''')
assert_fail('''b'abc'.partition(True)''')
assert_fail('''b'abc'.rpartition(True)''')
assert_fail('''b'abc'.partition(b'')''')
assert_fail('''b'abc'.rpartition(b'')''')

assert_eq(b'abc'.partition(b'banana'), (b'abc', b'', b''))
assert_eq(b'abc'.partition(b'a'), (b'', b'a', b'bc'))
assert_eq(b'abc'.partition(b'b'), (b'a', b'b', b'c'))
assert_eq(b'abc'.partition(b'c'), (b'ab', b'c', b''))
assert_eq(b'aaa'.partition(b'a'), (b'', b'a', b'aa'))
assert_eq(b'abcabcaabcabcabcaabc'.partition(b'aa'), (b'abcabc', b'aa', b'bcabcabcaabc'))

assert_eq(b'abc'.rpartition(b'banana'), (b'', b'', b'abc'))
assert_eq(b'abc'.rpartition(b'a'), (b'', b'a', b'bc'))
assert_eq(b'abc'.rpartition(b'b'), (b'a', b'b', b'c'))
assert_eq(b'abc'.rpartition(b'c'), (b'ab', b'c', b''))
assert_eq(b'aaa'.rpartition(b'a'), (b'aa', b'a', b''))
assert_eq(b'abcabcaabcabcabcaabc'.rpartition(b'aa'), (b'abcabcaabcabcabc', b'aa', b'bc'))

# TEST(StarlarkBytes, PartitionBytes) {
#   auto test = [](std::string_view element, std::string_view separator, std::string_view expected) {
#     error_handler error_callback;
#     Arena arena;
#     context ctx(arena);
#     starlark_bytes bytes(element);
#     starlark_bytes param1(separator);
# 
#     starlark_obj::pos_args_t pos_args;
#     starlark_obj::named_args_t named_args;
#     pos_args.push_back(&param1);
#     auto* method = bytes.dot("partition", ctx, error_callback);
#     ASSERT_NE(nullptr, method);
#     EXPECT_THAT(error_callback.messages, IsEmpty());
# 
#     auto* result = method->call(pos_args, named_args, ctx, error_callback);
#     ASSERT_NE(nullptr, result);
#     EXPECT_EQ(result->str(), expected);
#     ASSERT_THAT(error_callback.messages, IsEmpty());
#   };
# 
#   test("abc", "banana", "(b\"abc\", b\"\", b\"\")");
#   test("abc", "a", "(b\"\", b\"a\", b\"bc\")");
#   test("abc", "b", "(b\"a\", b\"b\", b\"c\")");
#   test("abc", "c", "(b\"ab\", b\"c\", b\"\")");
#   test("aaa", "a", "(b\"\", b\"a\", b\"aa\")");
# }
# 
# TEST(StarlarkBytes, PartitionWithNamedArguments) {
#   error_handler error_callback;
#   Arena arena;
#   context ctx(arena);
#   starlark_bytes bytes("abc"sv);
# 
#   starlark_obj::pos_args_t pos_args;
#   starlark_obj::named_args_t named_args;
#   pos_args.push_back(ctx.zero());
#   named_args.insert("x", ctx.zero());
#   auto* method = bytes.dot("partition", ctx, error_callback);
#   ASSERT_NE(nullptr, method);
#   EXPECT_THAT(error_callback.messages, IsEmpty());
# 
#   auto* result = method->call(pos_args, named_args, ctx, error_callback);
#   EXPECT_EQ(nullptr, result);
# 
#   ASSERT_THAT(error_callback.messages, SizeIs(1));
#   EXPECT_EQ(error_callback.messages[0], "TypeError: partition() takes no keyword arguments");
#   EXPECT_EQ(bytes.str(), "b\"abc\"");
# }
# 
# 
# TEST(StarlarkBytes, RpartitionBytes) {
#   auto test = [](std::string_view element, std::string_view separator, std::string_view expected) {
#     error_handler error_callback;
#     Arena arena;
#     context ctx(arena);
#     starlark_bytes bytes(element);
#     starlark_bytes param1(separator);
# 
#     starlark_obj::pos_args_t pos_args;
#     starlark_obj::named_args_t named_args;
#     pos_args.push_back(&param1);
#     auto* method = bytes.dot("rpartition", ctx, error_callback);
#     ASSERT_NE(nullptr, method);
#     EXPECT_THAT(error_callback.messages, IsEmpty());
# 
#     auto* result = method->call(pos_args, named_args, ctx, error_callback);
#     ASSERT_NE(nullptr, result);
#     EXPECT_EQ(result->str(), expected);
#     ASSERT_THAT(error_callback.messages, IsEmpty());
#   };
# 
#   test("abc", "banana", "(b\"\", b\"\", b\"abc\")");
#   test("abc", "a", "(b\"\", b\"a\", b\"bc\")");
#   test("abc", "b", "(b\"a\", b\"b\", b\"c\")");
#   test("abc", "c", "(b\"ab\", b\"c\", b\"\")");
#   test("aaa", "a", "(b\"aa\", b\"a\", b\"\")");
# }
# 
# TEST(StarlarkBytes, RpartitionWithNamedArguments) {
#   error_handler error_callback;
#   Arena arena;
#   context ctx(arena);
#   starlark_bytes bytes("abc"sv);
# 
#   starlark_obj::pos_args_t pos_args;
#   starlark_obj::named_args_t named_args;
#   pos_args.push_back(ctx.zero());
#   named_args.insert("x", ctx.zero());
#   auto* method = bytes.dot("rpartition", ctx, error_callback);
#   ASSERT_NE(nullptr, method);
#   EXPECT_THAT(error_callback.messages, IsEmpty());
# 
#   auto* result = method->call(pos_args, named_args, ctx, error_callback);
#   EXPECT_EQ(nullptr, result);
# 
#   ASSERT_THAT(error_callback.messages, SizeIs(1));
#   EXPECT_EQ(error_callback.messages[0], "TypeError: rpartition() takes no keyword arguments");
#   EXPECT_EQ(bytes.str(), "b\"abc\"");
# }
# 
# 
