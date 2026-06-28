def test1():
  a = "abc"
  a += "def"
  assert_eq(a, "abcdef")

test1()

assert_fail('''
def test1():
  a = "abc"
  a += ()

test1()
''')

assert_fail('''
def test1():
  a = "abc"
  a += b'def'

test1()
''')

