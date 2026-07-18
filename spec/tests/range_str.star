assert_eq(str(range(0, 0, 1)), 'range(0)')
assert_eq(str(range(0, 1, 1)), 'range(1)')
assert_eq(str(range(0, 0, -1)), 'range(0, 0, -1)')
assert_eq(str(range(0, 100, 1)), 'range(100)')
assert_eq(str(range(1, 100, 1)), 'range(1, 100)')
assert_eq(str(range(0, 100, -1)), 'range(0, 100, -1)')
assert_eq(str(range(0, 100, 1000)), 'range(0, 100, 1000)')
assert_eq(str(range(0, 100, -1000)), 'range(0, 100, -1000)')
assert_eq(str(range(100, 0, 1)), 'range(100, 0)')
assert_eq(str(range(100, 0, -1)), 'range(100, 0, -1)')
assert_eq(str(range(100, 0, 1000)), 'range(100, 0, 1000)')
assert_eq(str(range(100, 0, -1000)), 'range(100, 0, -1000)')


