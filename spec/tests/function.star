def fn():
  pass

a = lambda: True

assert_true(True if fn else False)
assert_true(True if a else False)


