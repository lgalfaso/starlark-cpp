def foo():
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
