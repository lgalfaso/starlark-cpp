# No iteration at top level.
assert_fail('''
for x in []:
  pass
''', allow_static_error = True)

# Single assignment at top level.
assert_fail('''
a = 1
a = 2
''', allow_static_error = True)

assert_succeed('''\uFEFF
# A program can start with a BOM character.
a = 1
''')

# There are no restrictions on the character after a literal.
# This does not match Python, but follows all Starlark implementations allows this.
assert_succeed('''1and 2''')
assert_succeed('''""and""''')

# Float literals edge cases.
assert_succeed('''0.0 + 0. + .0 + 1e10 + 1e+10 + 1e-10 + 1.1e10 + 1.1e+10 + 1.1e-10''')


# It is a static error if the floating point value is too large to be represented by a finite float value.
assert_fail('''1e309''', allow_static_error = True)

# `break` and `continue` can only happen within a `for`
assert_fail('''
def foo():
  continue
''', allow_static_error = True)
assert_fail('''
def foo():
  break
''', allow_static_error = True)


# `if` and `for` can only happen within a function.
assert_fail('''
if True:
  pass
''', allow_static_error = True)
assert_fail('''
for x in []:
  pass
''', allow_static_error = True)

# `return` can only happen within a function.
assert_fail('''
return None
''', allow_static_error = True)


assert_fail('''
## Begin module: "//:test1.bzl"
x = 1
## End module
## Main
def foo():
  load("//:test1.bzl", "x")
''', allow_static_error = True)

