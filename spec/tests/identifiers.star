assert_succeed('''
a = 1
b = 2
c3 = 3
''')

# `3c` is not a valid identifier.
assert_fail('''
a = 1
b = 2
3x = 3
''', allow_static_error = True)

# Def is a keyword
assert_fail('''
a, def = 1, 2
''', allow_static_error = True)

# B is not yet defined.
assert_fail('''
a = b
''', allow_static_error = True)

# This is using the same logic as Python.
assert_succeed('''
a = None
ﬁnally = 2
''')

# The two Omegas are different, one is \u2126 and the other is \u03A9, but both have the same NFKC representation.
# Unicode is hard, and Python rules around this are also hard.
assert_succeed('''
Ω = 'hello world'
print(Ω)
''')

