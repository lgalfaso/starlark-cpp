assert_false(True if range(0, 0, 1) else False)
assert_false(True if range(0, 0, -1) else False)
assert_true(True if range(0, 100, 1) else False)
assert_false(True if range(0, 100, -1) else False)
assert_true(True if range(0, 100, 1000) else False)
assert_false(True if range(0, 100, -1000) else False)
assert_false(True if range(100, 0, 1) else False)
assert_true(True if range(100, 0, -1) else False)
assert_false(True if range(100, 0, 1000) else False)
assert_true(True if range(100, 0, -1000) else False)

assert_true(not range(0))
assert_false(not range(10))

