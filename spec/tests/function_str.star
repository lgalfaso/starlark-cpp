def fn():
  pass

assert_eq(str(fn), "<function fn from main>")
assert_eq(str(lambda: True), "<function <lambda> from main>")
assert_eq(str([].append), "<built-in method append of list value>")
assert_eq(str(len), "<built-in function len>")


