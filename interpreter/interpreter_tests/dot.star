a = "wxyz"
assert_eq(a.count(""), 5)
c = "abc".count
assert_eq(c(""), 4)

assert_fail("""
"abc".count = 3
""", error_message = "'string' object attribute 'count' is read-only\n    1 | \"abc\".count = 3\n      | ~~~~~^^^^^^\n")
assert_fail("""
"abc".count += 3
""", error_message = "unsupported operand type(s) for +=: 'builtin_function_or_method' and 'int'\n    1 | \"abc\".count += 3\n      | ~~~~~~~~~~~~^^\n")
assert_fail("""
"abc".count -= 3
""", error_message = "unsupported operand type(s) for -=: 'builtin_function_or_method' and 'int'\n    1 | \"abc\".count -= 3\n      | ~~~~~~~~~~~~^^\n")
assert_fail("""
"abc".count *= 3
""", error_message = "unsupported operand type(s) for *=: 'builtin_function_or_method' and 'int'\n    1 | \"abc\".count *= 3\n      | ~~~~~~~~~~~~^^\n")
assert_fail("""
"abc".count /= 3
""", error_message = "unsupported operand type(s) for /=: 'builtin_function_or_method' and 'int'\n    1 | \"abc\".count /= 3\n      | ~~~~~~~~~~~~^^\n")
assert_fail("""
"abc".count //= 3
""", error_message = "unsupported operand type(s) for //=: 'builtin_function_or_method' and 'int'\n    1 | \"abc\".count //= 3\n      | ~~~~~~~~~~~~^^^\n")
assert_fail("""
"abc".count %= 3
""", error_message = "unsupported operand type(s) for %=: 'builtin_function_or_method' and 'int'\n    1 | \"abc\".count %= 3\n      | ~~~~~~~~~~~~^^\n")
assert_fail("""
"abc".count &= 3
""", error_message = "unsupported operand type(s) for &=: 'builtin_function_or_method' and 'int'\n    1 | \"abc\".count &= 3\n      | ~~~~~~~~~~~~^^\n")
assert_fail("""
"abc".count |= 3
""", error_message = "unsupported operand type(s) for |=: 'builtin_function_or_method' and 'int'\n    1 | \"abc\".count |= 3\n      | ~~~~~~~~~~~~^^\n")
assert_fail("""
"abc".count ^= 3
""", error_message = "unsupported operand type(s) for ^=: 'builtin_function_or_method' and 'int'\n    1 | \"abc\".count ^= 3\n      | ~~~~~~~~~~~~^^\n")
assert_fail("""
"abc".count <<= 3
""", error_message = "unsupported operand type(s) for <<=: 'builtin_function_or_method' and 'int'\n    1 | \"abc\".count <<= 3\n      | ~~~~~~~~~~~~^^^\n")
assert_fail("""
"abc".count >>= 3
""", error_message = "unsupported operand type(s) for >>=: 'builtin_function_or_method' and 'int'\n    1 | \"abc\".count >>= 3\n      | ~~~~~~~~~~~~^^^\n")
