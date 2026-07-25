assert_eq("".format(), "")
assert_eq("abc".format(), "abc")
assert_eq("{{".format(), "{")
assert_eq("}}".format(), "}")

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

