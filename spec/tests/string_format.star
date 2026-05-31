assert_eq("".format(), "")
assert_eq("abc".format(), "abc")
assert_eq("{{".format(), "{")
assert_eq("}}".format(), "}")


#     auto* result = str.format(pos_args, named_args, ctx, error_callback);
#     ASSERT_NE(nullptr, result) << error_callback.messages[0];
#     EXPECT_EQ(result->type(), starlark_types::string_t);
#     EXPECT_EQ(result->str(), expected);
#     EXPECT_THAT(error_callback.messages, IsEmpty());
#   };
#   auto test_with_error = [](std::string_view to_interpolate, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, std::string_view expected_error) {
#     Arena arena;
#     context ctx(arena);
#     error_handler error_callback;
#     starlark_string str(to_interpolate);
#   
#     auto* result = str.format(pos_args, named_args, ctx, error_callback);
#     ASSERT_EQ(nullptr, result);
#     ASSERT_THAT(error_callback.messages, SizeIs(1));
#     EXPECT_EQ(error_callback.messages[0], expected_error);
#   };
# 
#   Arena arena;
#   context ctx(arena);
#   error_handler error_callback;
# 
#   starlark_string def("def"sv);
#   starlark_obj::pos_args_t empty_pos;
#   starlark_obj::pos_args_t with_zero;
#   starlark_obj::pos_args_t with_zero_one;
#   starlark_obj::named_args_t empty_names;
#   starlark_obj::named_args_t with_abc;
#   starlark_obj::named_args_t with_dollar;
# 
#   with_zero.push_back(ctx.zero());
#   with_zero_one.push_back(ctx.zero());
#   with_zero_one.push_back(ctx.one());
#   with_abc.insert("abc", &def);
#   with_dollar.insert("$", &def);

assert_eq("{}".format(0), "0")
assert_eq("abc{}def".format(0), "abc0def")
assert_eq("abc{}def".format(0, 1), "abc0def")
assert_eq("abc{}def{}ghi".format(0, 1), "abc0def1ghi")
assert_eq("abc{0}def{1}ghi".format(0, 1), "abc0def1ghi")
assert_eq("abc{1}def{0}ghi".format(0, 1), "abc1def0ghi")
assert_eq("abc{0}def{0}ghi".format(0, 1), "abc0def0ghi")
assert_eq("abc{1}def{0}ghi".format(0, 1, abc = "def"), "abc1def0ghi")
assert_eq("xyz{abc}qwe".format(0, 1, abc = "def"), "xyzdefqwe")
assert_eq("abc{0000}def{1}ghi".format(0, 1), "abc0def1ghi")
assert_eq("{$}".format(**{'$': "def"}), "def")

assert_fail('''
"{".format()
''')

assert_fail('''
"}".format()
''')

assert_fail('''
"}abc".format()
''')

assert_fail('''
"{0".format()
''')

assert_fail('''
"{0a}".format(**{'0a': "abc"})
''')

assert_fail('''
"{}".format()
''')

assert_fail('''
"{0}".format()
''')

assert_fail('''
"{18446744073709551616}".format()
''')

assert_fail('''
"{abc}".format()
''')

assert_fail('''
"{:}".format(**{':': "abc"})
''')

assert_fail('''
"{a!}".format(**{'a!': "abc"})
''')


assert_fail('''
"{}{0}".format(0, 1, 2, 3)
''')

