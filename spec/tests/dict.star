assert_false(True if {} else False)
assert_true(True if {'a': 1} else False)
assert_true(not {})
assert_false(not {'a': 1})

# Extra comma is allowed.
assert_eq({None: None, }, {None: None})
# A comma without an entry is not.
assert_fail('''{,}''', allow_static_error = True)

# Fail is the same key is used multiple times.
assert_fail('''
{1: "foo", 1: "foo"}
''')

# Key and value expressions are evaluated left to right.
a = []
def p(x):
  a.append(x)
  return x

d = {p(1): p(2), p(3): p(4)}
assert_eq(a, [1, 2, 3, 4])

# Iteration order is deterministic.
r = []
def run():
  for x in {1: "", 2: "", 3: "", 5: "", 4: "", -1: ""}:
    r.append(x)
  assert_eq(r, [1, 2, 3, 5, 4, -1])
