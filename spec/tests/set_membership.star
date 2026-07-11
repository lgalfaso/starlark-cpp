
assert_true(1 in set([0, 1, 2]))
assert_false(1 in set([0, 2]))
assert_fail('''set() in set()''')

