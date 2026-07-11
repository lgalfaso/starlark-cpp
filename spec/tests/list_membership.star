assert_true(1 in [0, 1])
assert_false(2 in [0, 1])
assert_false(None in [0, 1])

r1 = []
r2 = [r1]
r1.append(r2)
assert_true(r1 in r1)

