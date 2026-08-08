assert_eq(0 <= 0, True)
assert_eq(0 <= 1, True)
assert_eq(1 <= 0, False)
assert_fail("a1 = False <= 1", error_message = "TypeError: '<=' not supported between instances of 'bool' and 'int'\n    1 | a1 = False <= 1\n      |            ^^\n")
