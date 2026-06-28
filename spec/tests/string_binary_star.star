a = ""
b = "abc"
assert_eq(b * 2, "abcabc")
assert_eq(b * 3, "abcabcabc")
assert_eq(b * -2, "")
assert_eq(b * -1, "")
assert_eq(a * (1 << 64), "")
assert_eq(a * 2, "")

assert_eq(2 * b, "abcabc")
assert_eq(3 * b, "abcabcabc")
assert_eq(-2 * b, "")
assert_eq(-1 * b, "")
assert_eq((1 << 64) * a, "")
assert_eq(2 * a, "")

assert_fail('''"abc" * ()''')
assert_fail('''"abc" * (1 << 64)''')

