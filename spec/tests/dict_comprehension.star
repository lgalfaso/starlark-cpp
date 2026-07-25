words = ["able", "baker", "charlie"]
assert_eq({x: len(x) for x in words}, {"charlie": 7, "baker": 5, "able": 4})


assert_fail('''{x: 2*x for x in 1, 2, 3}''', allow_static_error = True)
assert_fail('''{x: 2*x for x in lambda: 0}''', allow_static_error = True)
assert_fail('''{1:1 if True}''', allow_static_error = True)
assert_eq({1 if False else 2: 1}, {2:1})
assert_eq({1 if True else 2:1 for x in range(10)}, {1: 1})
assert_eq({x*y+z:1 for (x, y), z in [((2, 3), 5), (("o", 2), "!")]}, {11:1, 'oo!':1})
