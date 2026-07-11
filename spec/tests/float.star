assert_false(True if 0.0 else False)
assert_false(True if -0.0 else False)
assert_true(True if 1.0 else False)
assert_true(True if 1e308 * 100 else False)  # inf
assert_true(True if (1e308 * 100)/(1e308 * 100) else False)  # NaN

assert_succeed('''{1.0: None}''')
assert_eq({1.0: None}, {1: None})
