
assert_true(1 in {1: 2})
assert_false(1 not in {1: 2})
assert_false(2 in {1: 2})
assert_true(2 not in {1: 2})

assert_fail('''[] in {}''')
assert_fail('''[] not in {}''')

