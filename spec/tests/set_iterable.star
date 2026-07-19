
assert_eq([x for x in set([0, 1, 2, 3, 5, 4, 7, 6])], [0, 1, 2, 3, 5, 4, 7, 6])

# Elements are iterated in the insertion order.
# Attempting to add an element that is already present does not change the iteration order.
a = set([0, 1])
a.add(3)
a.add(2)
a.add(1)
assert_eq([x for x in a], [0, 1, 3, 2])


assert_fail('''
def test():
  a = set([0])
  for x in a:
    a.update([x])

test()
''')

