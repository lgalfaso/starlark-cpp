assert_fail('''['a', 'b', 'c'][-4]''')
l = ['a', 'b', 'c']
assert_eq(l[-3], 'a')
assert_eq(l[-2], 'b')
assert_eq(l[-1], 'c')
assert_eq(l[0], 'a')
assert_eq(l[1], 'b')
assert_eq(l[2], 'c')
assert_fail('''['a', 'b', 'c'][3]''')
assert_fail('''['a', 'b', 'c'][1 << 64]''')
assert_fail('''['a', 'b', 'c'][None]''')
assert_fail('''['a', 'b', 'c'][False]''')

