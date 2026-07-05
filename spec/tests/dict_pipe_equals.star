def test1():
  d1 = {0: "zero", 1: "one"}
  d2 = {0: "four", 2: "two", 3: "three"}
  d1 |= d2
  assert_eq(d1, {0: "four", 1: "one", 2: "two", 3: "three"})

test1()

def test2():
  d1 = {0: "zero", 1: "one"}
  d1 |= d1
  assert_eq(d1, {0: "zero", 1: "one"})

test2()

assert_fail('''
def test3():
  d1 = {0: "zero", 1: "one"}
  d1 |= []

test3()
''')


assert_fail('''
def test4():
  d1 = {0: "zero", 1: "one"}
  for x in d1:
    d1 |= d1

test4()
''')


assert_fail('''
## Begin module: "//:test1.bzl"
d1 = {0: "zero", 1: "one"}
## End module
## Main
load("//:test1.bzl", "d1")

def test5():
  d2 = d1
  d2 |= {}

test5()
''')


