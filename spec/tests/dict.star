assert_false(True if {} else False)
assert_true(True if {'a': 1} else False)

assert_fail('''
{1: "foo", 1: "foo"}
''')

# Iteration order is deterministic.
r = []
def run():
  for x in {1: "", 2: "", 3: "", 5: "", 4: "", -1: ""}:
    r.append(x)
  assert_eq(r, [1, 2, 3, 5, 4, -1])
