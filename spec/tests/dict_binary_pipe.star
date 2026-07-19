assert_eq({0: "zero", 1: "one"} | {0: "four", 2: "two", 3: "three"}, {0: "four", 1: "one", 2: "two", 3: "three"})
assert_eq(str({None: 1} | {None: 1.0}), '{None: 1.0}')
assert_eq(str({1: 1} | {1.0: 1.0}), '{1: 1.0}')

assert_fail('''{} | ()''')

