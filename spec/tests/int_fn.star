# No arguments.
assert_eq(int(), 0)

# One argument.
assert_eq(int(42), 42)
assert_eq(int(1e70), 10000000000000000725314363815292351261583744096465219555182101554790400)
assert_fail('''int(1e308 * 10)''')  # Inf.
assert_fail('''
inf = 1e308 * 10
nan = inf/inf
int(nan)''')
assert_eq(int(False), 0)
assert_eq(int(True), 1)
assert_eq(int("-0123"), -123)
assert_eq(int("+0123"), 123)
assert_eq(int("-123"), -123)
assert_eq(int("0"), 0)
assert_eq(int("123"), 123)
assert_eq(int("0123"), 123)
assert_fail('''int('123abc')''')
assert_fail('''int('')''')
assert_fail('''int([])''')
assert_fail('''int(base = 10)''')


# Two arguments.
assert_fail('''int(42, 10)''')
assert_fail('''int(42, base = 10)''')
assert_fail('''int(42.0, 10)''')
assert_fail('''int(42.0, base = 10)''')
assert_fail('''int(False, 10)''')
assert_fail('''int(False, base = 10)''')
assert_eq(int("123", 10), 123)
assert_eq(int("123", 0), 123)
assert_eq(int("0123", 10), 123)
assert_eq(int("0x123", 16), 291)
assert_eq(int("+0x123", 16), 291)
assert_eq(int("-0x123", 16), -291)
assert_eq(int("123", 16), 291)
assert_eq(int("0x123", 0), 291)
assert_eq(int("0o123", 8), 83)
assert_eq(int("-0o123", 8), -83)
assert_eq(int("+0o123", 8), 83)
assert_eq(int("123", 8), 83)
assert_eq(int("0o123", 0), 83)
assert_eq(int("0b101", 2), 5)
assert_eq(int("+0b101", 2), 5)
assert_eq(int("-0b101", 2), -5)
assert_eq(int("101", 2), 5)
assert_eq(int("0b101", 0), 5)
assert_fail('''int('1', -1)''')
assert_fail('''int('1', base = -1)''')
assert_fail('''int('1', 1)''')
assert_fail('''int('1', base = 1)''')
assert_fail('''int('1', 37)''')
assert_fail('''int('1', base = 37)''')
assert_fail('''int('1', 100)''')
assert_fail('''int('1', base = 100)''')
assert_fail('''int('1', None)''')
assert_fail('''int('1', True)''')
assert_fail('''int('1', base = None)''')
assert_fail('''int('1', base = True)''')


# Many arguments.
assert_fail('''int('1', 10, 10)''')
assert_fail('''int('1', 10, base = 10)''')


# Named arguments.
assert_fail('''int(string = '1', base = True)''')
assert_fail('''int(number = 1)''')

