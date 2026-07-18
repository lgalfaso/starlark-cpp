assert_false(1 in ())
assert_false(1 in (0,))
assert_true(0 in (0,))
assert_true(0 in (2, 0))

