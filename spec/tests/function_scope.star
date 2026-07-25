
def outter1(x):
  def inner1():
    x = 1  # This will not hide the outter definition.
  def inner2():
    return x
  return (inner1, inner2)

[a1, b1] = outter1('hello')
assert_eq(b1(), 'hello')
a1()
assert_eq(b1(), 'hello')



def outter2(x):
  def inner1():
    x[0] = 1  # This will change the outter variable.
  def inner2():
    return x
  return (inner1, inner2)

[a2, b2] = outter2(['hello'])
assert_eq(b2(), ['hello'])
a2()
assert_eq(b2(), [1])

assert_fail('''
## Begin module: "//:test1.bzl"
def outter3(x):
  def inner1():
    x[0] = 1  # This will change the outter variable.
  def inner2():
    return x
  return (inner1, inner2)

[a3, b3] = outter3(['hello'])
## End module
## Main
load("//:test1.bzl", "a3", "b3")
b3()
a3()
''')

