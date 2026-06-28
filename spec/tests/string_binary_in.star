assert_true("" in "")
assert_true("" in "a")
assert_false("b" in "a")

assert_fail('''97 in "a"''')
assert_fail('''98 in "a"''')
assert_fail('''(1 << 64) in "a"''')
assert_fail('''-1 in "a"''')
assert_fail('''256 in "a"''')

