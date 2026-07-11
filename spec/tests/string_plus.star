assert_eq("" + "", "")
assert_eq("abc" + "", "abc")
assert_eq("" + "def", "def")
assert_eq("abc" + "def", "abcdef")
assert_succeed("""
a = "abc" + "def"
""")
assert_succeed("""
a = ("abc" +
     "def")
""")
assert_fail("""
a = "abc" +
     "def"
""", allow_static_error = True)
assert_fail("""
"abc" + b"def"
""")
