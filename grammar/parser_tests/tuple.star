a = (1,)
b = (1,2,3)

def foo():
  for (a,) in [(1,)]:
    print(a)

c = ()
d = (())
e = ((1,))

