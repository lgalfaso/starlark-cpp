a = "wxyz"
assert_eq(a.count(), 4)
c = "abc".count
assert_eq(c(), 3)

assert_fail("""
"abc".count = 3
""", error_message = "AttributeError: 'string' object attribute 'count' is read-only")
