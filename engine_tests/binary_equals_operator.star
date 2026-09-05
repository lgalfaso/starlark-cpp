assert_eq([] == [], True)
assert_eq([] == False, False)
assert_eq(True != [], True)
assert_eq(False != False, False)

def bar():
  def foo(): pass
  return foo

def foo(): pass

assert_ne(foo, bar())
assert_ne(bar(), bar())
