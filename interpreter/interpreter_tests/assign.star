a = 1
assert_eq(a, 1)
a += 2
assert_eq(a, 3)
a *= 5
assert_eq(a, 15)
a <<= 2
assert_eq(a, 60)
a >>= 1
assert_eq(a, 30)
a //= 3
assert_eq(a, 10)
a %= 3
assert_eq(a, 1)
a /= 3
assert_eq(a, 1/3)
a = 12
a &= 4
assert_eq(a, 4)
a ^= 8
assert_eq(a, 12)

b = {1: '1', 2: '2', 3: '3'}
b |= {2: '22', 4: '4'}
b |= b
assert_eq('{1: "1", 2: "22", 3: "3", 4: "4"}', repr(b))

c = set([1,2,3,4])
c -= c

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
