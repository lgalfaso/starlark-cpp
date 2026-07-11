assert_eq(str(''), '')
assert_eq(str('abc'), 'abc')

assert_eq(repr(''), '""')
assert_eq(repr('abc'), '"abc"')
assert_eq(repr('\''), '"\'"')
assert_eq(repr('"'), '"\\""')
assert_eq(repr('\t\r\n'), '"\\t\\r\\n"')
assert_eq(repr('\001\002\177'), '"\\x01\\x02\\x7f\"')
assert_eq(repr('\u0378'), '"\\u0378"')
assert_eq(repr('\U000101c7'), '"\\U000101c7"')
assert_eq(repr('🙂'), '"🙂"')
assert_eq(repr('\ufeff'), '"\\ufeff"')

assert_fail('''"\\x90"''', allow_static_error = True)
assert_fail('''"\\ud800"''', allow_static_error = True)

