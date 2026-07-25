def foo_01(a, b):
  pass
def foo_03(a, *b):
  pass
def foo_04(a, **b):
  pass
def foo_05(*, b):
  pass
def foo_06(*, b, **a):
  pass
def foo_09(*a, b):
  pass
def foo_12(*a, **b):
  pass
def foo_17(a, *, b):
  pass
def bar_01(bar_01 = lambda bar_01: True):
  pass
def g(a, *args, b, c = 2):
  print(a, b, c, args)
def h(a = [], *args, b = 1, c = 2):
  pass
def i(y = [], *, z):
  pass
def j(x, y = [], *, w, z = []):
  pass
def k(x, y = [], *, w = [], z):
  pass
def l(x, y = [], *args, w = [], z):
  pass
