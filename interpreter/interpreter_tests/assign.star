a = 1
assert_eq(a, 1)

assert_fail("""
a = list(range(3))
a[:] = range(3, 6)
""", error_message = "TypeError: 'list' object does not support item assignment")
assert_fail("""
a = list(range(3))
a[:] += range(3, 6)
""", error_message = "TypeError: 'list' object does not support item assignment")
assert_fail("""
a = list(range(3))
a[:] -= range(3, 6)
""", error_message = "TypeError: 'list' object does not support item assignment")
assert_fail("""
a = list(range(3))
a[:] *= range(3, 6)
""", error_message = "TypeError: 'list' object does not support item assignment")
assert_fail("""
a = list(range(3))
a[:] /= range(3, 6)
""", error_message = "TypeError: 'list' object does not support item assignment")
assert_fail("""
a = list(range(3))
a[:] //= range(3, 6)
""", error_message = "TypeError: 'list' object does not support item assignment")
assert_fail("""
a = list(range(3))
a[:] %= range(3, 6)
""", error_message = "TypeError: 'list' object does not support item assignment")
assert_fail("""
a = list(range(3))
a[:] &= range(3, 6)
""", error_message = "TypeError: 'list' object does not support item assignment")
assert_fail("""
a = list(range(3))
a[:] |= range(3, 6)
""", error_message = "TypeError: 'list' object does not support item assignment")
assert_fail("""
a = list(range(3))
a[:] ^= range(3, 6)
""", error_message = "TypeError: 'list' object does not support item assignment")
assert_fail("""
a = list(range(3))
a[:] <<= range(3, 6)
""", error_message = "TypeError: 'list' object does not support item assignment")
assert_fail("""
a = list(range(3))
a[:] >>= range(3, 6)
""", error_message = "TypeError: 'list' object does not support item assignment")
