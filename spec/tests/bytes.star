assert_false(True if b'' else False)
assert_true(True if b'abc' else False)

assert_fail('''
def foo():
  for x in b"":
    pass

foo()
''')
