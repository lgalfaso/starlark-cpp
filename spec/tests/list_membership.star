assert_true(1 in [0, 1])
assert_false(1 not in [0, 1])
assert_false(2 in [0, 1])
assert_true(2 not in [0, 1])
assert_false(None in [0, 1])
assert_true(None not in [0, 1])

r1 = []
r2 = [r1]
r1.append(r2)
assert_true(r1 in r1)
assert_false(r1 not in r1)

