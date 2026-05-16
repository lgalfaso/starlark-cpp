a = set([1, 2])
assert_false(a.issubset([1]))
assert_true(a.issubset([1, 2]))
assert_true(a.issubset([1, 2, 3]))
assert_fail("""
a = set([1, 2])
a.issubset([1, 2, set()])""")
assert_fail("""
a = set([1, 2])
a.issubset([set()])""")

