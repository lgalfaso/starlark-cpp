def test1():
  a = []
  for x in range(10):
     a.append(x)
  assert_eq(a, [0, 1, 2, 3, 4, 5, 6, 7, 8, 9])

test1()


def test2():
  a = []
  b = []
  for x, y in [["a", 1], ["b", 2], ["c", 3]]:
    a.append(x)
    b.append(y)
  assert_eq(a, ['a', 'b', 'c'])
  assert_eq(b, [1, 2, 3])

test2()


assert_fail('''
for a in []: pass
''', allow_static_error = True)


def test3():
  a = []
  for x in range(10):
     a.append(x)
     if x == 5: break;
     a.append(x)
  assert_eq(a, [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5])

test3()


def test4():
  a = []
  for x in range(10):
     a.append(x)
     if x % 2 == 0: continue;
     a.append(x)
  assert_eq(a, [0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 7, 7, 8, 9, 9])

test4()


assert_fail('''
break
''', allow_static_error = True)
assert_fail('''
continue
''', allow_static_error = True)

assert_fail('''
def test():
  if True: break
''', allow_static_error = True)
assert_fail('''
def test():
  if True: continue
''', allow_static_error = True)

assert_fail('''
def test():
  for x in []:
    def foo():
      break
''', allow_static_error = True)
assert_fail('''
def test():
  for x in []:
    def foo():
      continue
''', allow_static_error = True)
