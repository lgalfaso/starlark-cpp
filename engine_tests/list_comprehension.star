assert_eq([x*x for x in [1,2,3,4]], [1, 4, 9, 16])
assert_eq([x*y for x in [1,2,3,4] for y in [5, 6]], [5, 6, 10, 12, 15, 18, 20, 24])
assert_eq([x*y for x in [1,2,3,4] if x % 2 == 1 for y in [5, 6]], [5, 6, 15, 18])

