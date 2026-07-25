def test1(x):
  if x > 0:
    return "positive"

assert_eq(test1(-1), None)
assert_eq(test1(0), None)
assert_eq(test1(1), "positive")


def test2(x):
  if x > 0:
    return "positive"
  else:
    return "non-negative"

assert_eq(test2(-1), "non-negative")
assert_eq(test2(0), "non-negative")
assert_eq(test2(1), "positive")

def test3(x):
  if x > 0:
    return "positive"
  elif x < 0:
    return "negative"
  else:
    return "zero"

assert_eq(test3(-1), "negative")
assert_eq(test3(0), "zero")
assert_eq(test3(1), "positive")


assert_fail('''
if True:
  print("hello")
''', allow_static_error = True)
