assert_false(1 in ())
assert_true(1 not in ())
assert_false(1 in (0,))
assert_true(1 not in (0,))
assert_true(0 in (0,))
assert_false(0 not in (0,))
assert_true(0 in (2, 0))
assert_false(0 not in (2, 0))

