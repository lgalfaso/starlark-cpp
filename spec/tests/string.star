assert_false(True if "" else False)
assert_true(True if "abc" else False)

assert_fail('''
def foo():
  for x in "":
    pass

foo()
''')
