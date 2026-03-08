a = "wxyz"
assert_eq(a.count(""), 5)
c = "abc".count
assert_eq(c(""), 4)

assert_fail("""
"abc".count = 3
""", error_message = "AttributeError: 'string' object attribute 'count' is read-only")
assert_fail("""
"abc".count += 3
""", error_message = "TypeError: unsupported operand type(s) for +=: 'builtin_function_or_method' and 'int'")
assert_fail("""
"abc".count -= 3
""", error_message = "TypeError: unsupported operand type(s) for -=: 'builtin_function_or_method' and 'int'")
assert_fail("""
"abc".count *= 3
""", error_message = "TypeError: unsupported operand type(s) for *=: 'builtin_function_or_method' and 'int'")
assert_fail("""
"abc".count /= 3
""", error_message = "TypeError: unsupported operand type(s) for /=: 'builtin_function_or_method' and 'int'")
assert_fail("""
"abc".count //= 3
""", error_message = "TypeError: unsupported operand type(s) for //=: 'builtin_function_or_method' and 'int'")
assert_fail("""
"abc".count %= 3
""", error_message = "TypeError: unsupported operand type(s) for %=: 'builtin_function_or_method' and 'int'")
assert_fail("""
"abc".count &= 3
""", error_message = "TypeError: unsupported operand type(s) for &=: 'builtin_function_or_method' and 'int'")
assert_fail("""
"abc".count |= 3
""", error_message = "TypeError: unsupported operand type(s) for |=: 'builtin_function_or_method' and 'int'")
assert_fail("""
"abc".count ^= 3
""", error_message = "TypeError: unsupported operand type(s) for ^=: 'builtin_function_or_method' and 'int'")
assert_fail("""
"abc".count <<= 3
""", error_message = "TypeError: unsupported operand type(s) for <<=: 'builtin_function_or_method' and 'int'")
assert_fail("""
"abc".count >>= 3
""", error_message = "TypeError: unsupported operand type(s) for >>=: 'builtin_function_or_method' and 'int'")
