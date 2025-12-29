assert_eq({x: x*x for x in [1,2,3,4]}, {1: 1, 2: 4, 3: 9, 4: 16})
assert_eq({x: y for x in [1,2,3,4] for y in [5, 6]}, {1: 6, 2: 6, 3: 6, 4: 6})
assert_eq({x: y for x in [1,2,3,4] if x % 2 == 1 for y in [5, 6]}, {1: 6, 3: 6})

