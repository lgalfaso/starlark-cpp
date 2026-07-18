# No arguments.
assert_fail('''chr()''')


# One argument.
assert_eq(chr(0), '\x00')
assert_eq(chr(65), 'A')
assert_eq(chr(256), '\u0100')
assert_eq(chr(0x10ffff), '\U0010ffff')

assert_fail('''chr(-1)''')
assert_fail('''chr(0x110000)''')
assert_fail('''chr(0.0)''')
assert_fail('''chr(None)''')
assert_fail('''chr(True)''')
assert_fail('''chr([])''')


# Named arguments.
assert_fail('''chr(codepoint = 65)''')

