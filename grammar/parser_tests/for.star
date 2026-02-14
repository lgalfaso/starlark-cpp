def foo():
  x = 1
  for a in [x]:
    pass
  for a, b in x:
    pass
  for a in x:
    break
  for a in x:
    continue
  for (a) in x:
    continue
  for a.b in x:
    continue
  for (a, b) in x:
    continue
  for a in [1,]:
    continue
  for (a,) in [(1,)]:
    continue
  for (a,b),c in []:
    continue
  for x in 1, 2:
    print(x)
