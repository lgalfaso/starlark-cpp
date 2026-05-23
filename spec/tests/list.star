assert_false(True if [] else False)
assert_true(True if [0] else False)
assert_fail("""
{[]: None}""")

# List is a subtype of Sequence.
def foo():
  b = 0
  for a in [1, 2, 3]:
    b += a;
  return b

assert_eq(foo(), 6)
