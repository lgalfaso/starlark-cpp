assert_eq({0: "zero", 1: "one"} | {0: "four", 2: "two", 3: "three"}, {0: "four", 1: "one", 2: "two", 3: "three"})

assert_fail('''{} | ()''')

