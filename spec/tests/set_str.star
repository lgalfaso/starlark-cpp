assert_eq(str(set()), 'set()')
assert_eq(str(set([None])), 'set([None])')
assert_eq(str(set([None, True])), 'set([None, True])')
assert_eq(str(set([None, True, 1])), 'set([None, True, 1])')

