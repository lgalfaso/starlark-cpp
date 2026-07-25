assert_false(True if [] else False)
assert_true(True if [0] else False)
assert_true(not [])
assert_false(not [0])


assert_fail("""
{[]: None}""")

# List may have a trailing comma.
a = [1,]
# but not empty lists.
assert_fail('''[,]''', allow_static_error = True)

# List is a subtype of Sequence.
def foo():
  b = 0
  for a in [1, 2, 3]:
    b += a;
  return b

assert_eq(foo(), 6)


assert_fail('''+[]''')
assert_fail('''-[]''')
assert_fail('''~[]''')

# List are not hashable.
assert_fail('''
{[]: 1}
''')
# Inmutable lists are also not hashable.
assert_fail('''
## Begin module: "//:test1.bzl"
a = [0, 1, 2]
## End module
## Main
load("//:test1.bzl", "a")
{a: None}
''')
