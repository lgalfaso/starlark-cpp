
assert_true(1 in {1: 2})
assert_false(2 in {1: 2})

assert_fail('''[] in {}''')

