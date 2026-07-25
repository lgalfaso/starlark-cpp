def test1():
  a = []
  for x in 1, 2:
    a.append(x)
  assert_eq(a, [1, 2])

test1()
