def test1():
  empty = ""
  abc = "abc"
  x = abc
  x *= 2
  assert_eq(x, "abcabc")
  x = abc
  x *= 3
  assert_eq(x, "abcabcabc")
  x = abc
  x *= -2
  assert_eq(x, "")
  x = abc
  x *= -1
  assert_eq(x, "")
  assert_eq(abc, "abc")
  empty *= 1 << 64
  assert_eq(empty, "")
  empty *= 2
  assert_eq(empty, "")

test1()


def test2():
  a = 2
  a *= "abc"
  assert_eq(a, "abcabc")
  a = 3
  a *= "abc"
  assert_eq(a, "abcabcabc")
  a = -2
  a *= "abc"
  assert_eq(a, "")
  a = -1
  a *= "abc"
  assert_eq(a, "")
  a = 1 << 64
  a *= ""
  assert_eq(a, "")
  a = 2
  a *= ""
  assert_eq(a, "")

test2()
  

assert_fail('''
def foo():
  a = "abc"
  a *= ()

foo()
''')

