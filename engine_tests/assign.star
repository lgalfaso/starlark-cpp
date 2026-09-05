def run():
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

  d = list(range(15))
  d[2] += 100
  d[3] -= 100
  d[4] *= 100
  d[5] /= 3
  d[6] //= 3
  d[7] %= 3
  d[8] |= 3
  d[9] &= 3
  d[10] ^= 3
  d[11] <<= 3
  d[12] >>= 3
  assert_eq(d, [0, 1, 102, -97, 400, 5/3, 2, 1, 11, 1, 9, 88, 1, 13, 14])

  assert_fail("""
def exec():
  a = list(range(3))
  a[:] = range(3, 6)
exec()
  """, error_message = "'list' object does not support slice assignment\n    3 |   a[:] = range(3, 6)\n      |   ~^^^\n")

  assert_fail("""
def exec():
  a = list(range(3))
  a[:] += range(3, 6)
exec()
  """, error_message = "'list' object does not support slice assignment\n    3 |   a[:] += range(3, 6)\n      |   ~^^^\n")

  assert_fail("""
def exec():
  a = list(range(3))
  a[:] -= range(3, 6)
exec()
  """, error_message = "'list' object does not support slice assignment\n    3 |   a[:] -= range(3, 6)\n      |   ~^^^\n")

  assert_fail("""
def exec():
  a = list(range(3))
  a[:] *= range(3, 6)
exec()
  """, error_message = "'list' object does not support slice assignment\n    3 |   a[:] *= range(3, 6)\n      |   ~^^^\n")

  assert_fail("""
def exec():
  a = list(range(3))
  a[:] /= range(3, 6)
exec()
  """, error_message = "'list' object does not support slice assignment\n    3 |   a[:] /= range(3, 6)\n      |   ~^^^\n")

  assert_fail("""
def exec():
  a = list(range(3))
  a[:] //= range(3, 6)
exec()
  """, error_message = "'list' object does not support slice assignment\n    3 |   a[:] //= range(3, 6)\n      |   ~^^^\n")

  assert_fail("""
def exec():
  a = list(range(3))
  a[:] %= range(3, 6)
exec()
  """, error_message = "'list' object does not support slice assignment\n    3 |   a[:] %= range(3, 6)\n      |   ~^^^\n")

  assert_fail("""
def exec():
  a = list(range(3))
  a[:] &= range(3, 6)
exec()
  """, error_message = "'list' object does not support slice assignment\n    3 |   a[:] &= range(3, 6)\n      |   ~^^^\n")

  assert_fail("""
def exec():
  a = list(range(3))
  a[:] |= range(3, 6)
exec()
  """, error_message = "'list' object does not support slice assignment\n    3 |   a[:] |= range(3, 6)\n      |   ~^^^\n")

  assert_fail("""
def exec():
  a = list(range(3))
  a[:] ^= range(3, 6)
exec()
  """, error_message = "'list' object does not support slice assignment\n    3 |   a[:] ^= range(3, 6)\n      |   ~^^^\n")

  assert_fail("""
def exec():
  a = list(range(3))
  a[:] <<= range(3, 6)
exec()
  """, error_message = "'list' object does not support slice assignment\n    3 |   a[:] <<= range(3, 6)\n      |   ~^^^\n")

  assert_fail("""
def exec():
  a = list(range(3))
  a[:] >>= range(3, 6)
exec()
  """, error_message = "'list' object does not support slice assignment\n    3 |   a[:] >>= range(3, 6)\n      |   ~^^^\n")

run()
