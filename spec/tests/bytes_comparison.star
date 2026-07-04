assert_false(b'' < b'')
assert_true(b'' <= b'')
assert_true(b'' >= b'')
assert_false(b'' > b'')

assert_true(b'' < b'a')
assert_true(b'' <= b'a')
assert_false(b'' >= b'a')
assert_false(b'' > b'a')

assert_false(b'a' < b'a')
assert_true(b'a' <= b'a')
assert_true(b'a' >= b'a')
assert_false(b'a' > b'a')

assert_false(b'a' < b'')
assert_false(b'a' <= b'')
assert_true(b'a' >= b'')
assert_true(b'a' > b'')

assert_true(b'a' < b'b')
assert_true(b'a' <= b'b')
assert_false(b'a' >= b'b')
assert_false(b'a' > b'b')

assert_false(b'b' < b'a')
assert_false(b'b' <= b'a')
assert_true(b'b' >= b'a')
assert_true(b'b' > b'a')

assert_fail('''b'' < '' ''')
assert_fail('''b'' <= '' ''')
assert_fail('''b'' > '' ''')
assert_fail('''b'' >= '' ''')

