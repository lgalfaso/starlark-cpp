a = {0: "zero", 1: "one"}
assert_eq(a.clear(), None)
assert_eq(a, {})

assert_fail('''
def foo():
  a = {0: "zero", 1: "one"}
  for x in a:
    a.clear()

foo()
''')

assert_fail('''{}.clear(None)''')



