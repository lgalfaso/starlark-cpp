def test1():
  empty = b''
  abc = b'abc'
  x = abc
  x *= 2
  assert_eq(x, b'abcabc')
  x = abc
  x *= 3
  assert_eq(x, b'abcabcabc')
  x = abc
  x *= -2
  assert_eq(x, b'')
  x = abc
  x *= -1
  assert_eq(x, b'')
  assert_eq(abc, b'abc')
  empty *= 1 << 64
  assert_eq(empty, b'')
  empty *= 2
  assert_eq(empty, b'')

test1()


def test2():
  a = 2
  a *= b'abc'
  assert_eq(a, b'abcabc')
  a = 3
  a *= b'abc'
  assert_eq(a, b'abcabcabc')
  a = -2
  a *= b'abc'
  assert_eq(a, b'')
  a = -1
  a *= b'abc'
  assert_eq(a, b'')
  a = 1 << 64
  a *= b''
  assert_eq(a, b'')
  a = 2
  a *= b''
  assert_eq(a, b'')

test2()


assert_fail('''
def foo():
  a = b'abc'
  a *= ()

foo()
''')

