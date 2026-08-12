# Op in loop
def foo():
  a = [0, 1, 2]
  for x in a:
    a[1] = 3
foo()

