a = [1, 2]
b = [3, 4]
assert_eq(a + b, [1, 2, 3, 4])
assert_eq(a, [1, 2])
assert_eq(b, [3, 4])

assert_fail('''[] + ()''')

# The spec does not mandate a limit on the size, then there is no test that checks for such limit.

