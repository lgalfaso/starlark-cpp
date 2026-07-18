assert_eq([x for x in range(10, 20, -3)], [])
assert_eq([x for x in range(10, 20, 3)], [10, 13, 16, 19])
assert_eq([x for x in range(20, 10, -3)], [20, 17, 14, 11])

