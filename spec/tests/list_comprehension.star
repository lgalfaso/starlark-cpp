assert_eq([x*x for x in [1, 2, 3, 4]], [1, 4, 9, 16])

assert_fail('''[2*x for x in 1, 2, 3]''', allow_static_error = True)
assert_fail('''[2*x for x in lambda: 0]''', allow_static_error = True)
assert_fail('''[1 if True]''', allow_static_error = True)
assert_eq([1 if False else 2], [2])
assert_eq([1 if True else 2 for x in range(10)], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1])
assert_eq([x*y+z for (x, y), z in [((2, 3), 5), (("o", 2), "!")]], [11, 'oo!'])
