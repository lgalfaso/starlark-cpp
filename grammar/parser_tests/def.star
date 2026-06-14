def foo():
    x = True
    pass

def bar(x): True

def man(x = True):
  print(x)

def baz(*, a, b):
  b = 1
  a = 1

def shell(x = man):
  print(x)

def zoo(**x):
  pass
