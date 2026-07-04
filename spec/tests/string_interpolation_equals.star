# All the tests are equivalent to the normal percent operator.

def test():
  a = "abc %s def"
  a %= "xyz"
  assert_eq(a, "abc xyz def")

test()

