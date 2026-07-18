# No arguments.
assert_fail('''sorted()''')


# One argument.
assert_eq(sorted(()), [])
a = []
b = sorted(a)
assert_eq(b, [])
a.append(None)
assert_eq(b, [])
c = ['one', 'two', 'three', 'four', 'five', 'six', 'seven']
assert_eq(sorted(c), ["five", "four", "one", "seven", "six", "three", "two"])
assert_fail('''sorted(True)''')
assert_fail('''sorted(None)''')
assert_fail('''sorted(1)''')
assert_fail('''sorted([1, True])''')


# Two arguments.
assert_fail('''sorted([], None)''')


# Named arguments.
assert_eq(sorted(c, key = None), ["five", "four", "one", "seven", "six", "three", "two"])
assert_eq(sorted(c, key = len), ["one", "two", "six", "four", "five", "three", "seven"])
assert_eq(sorted(c, reverse = False), ["five", "four", "one", "seven", "six", "three", "two"])
assert_eq(sorted(c, key = None, reverse = False), ["five", "four", "one", "seven", "six", "three", "two"])
assert_eq(sorted(c, key = len, reverse = False), ["one", "two", "six", "four", "five", "three", "seven"])
assert_eq(sorted(c, reverse = True), ["two", "three", "six", "seven", "one", "four", "five"])
assert_eq(sorted(c, key = None, reverse = True), ["two", "three", "six", "seven", "one", "four", "five"])
assert_eq(sorted(c, key = len, reverse = True), ["seven", "three", "five", "four", "six", "two", "one"])
assert_eq(c, ['one', 'two', 'three', 'four', 'five', 'six', 'seven'])

assert_fail('''sorted([], reverse = None)''')
assert_fail('''sorted([], reverse = 1)''')
assert_fail('''sorted([], reverse = [])''')
assert_fail('''sorted(iterable = [])''')
assert_fail('''sorted([[], []], key = set)''')

