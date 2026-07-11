# Tuple is true if non-empty.
assert_false(True if () else False)
assert_true(True if (None,) else False)
assert_true(True if (1,) else False)

# Trailing comma is only allowed in parenthesized tuple expression.
assert_fail("""
def test():
  for k, v, in dict().items(): pass
""", allow_static_error = True)
assert_fail("""
_ = [(v, k) for k, v, in dict().items()]
""", allow_static_error = True)
assert_succeed("""
_ = [(v, k) for (k, v,) in dict().items()]
""")

# Tuples are hashable if their elements are hashable.
assert_succeed("""{(): None}""")
assert_succeed("""{(0,): None}""")
assert_succeed("""{(0, 1): None}""")
assert_fail("""{(0, []): None}""")

# Tuple concatenation.
assert_eq((1, 2) + (3, 4, 5), (1, 2, 3, 4, 5))
assert_fail("""() + []""")
assert_fail("""[] + ()""")

# Tuples are sequences.
def foo():
  r = 0
  for a in (1, 2, 3):
    r += a
  return r
assert_eq(foo(), 6)

# Tuples cannot be modified.
assert_fail("""
a = (1,)
a[0] = 2""")
