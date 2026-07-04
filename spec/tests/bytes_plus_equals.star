def test():
  a = b'abc'
  a += b'def'
  assert_eq(a, b'abcdef')

test()

assert_fail('''
def test():
  a = b'abc'
  a += ()

test()
''')

assert_fail('''
def test():
  a = b'abc'
  a += 'def'

test()
''')

