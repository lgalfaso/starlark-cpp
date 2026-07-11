assert_false(True if set() else False)
assert_true(True if set([1]) else False)
assert_eq(set([1]), set((1,)))
