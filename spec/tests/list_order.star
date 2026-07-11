all_elements = [[], [0], [0, 1], [1], [1, 0]]

def test():
  for x in range(len(all_elements)):
    for y in range(len(all_elements)):
      assert_eq(all_elements[x] < all_elements[y], x < y)
      assert_eq(all_elements[x] <= all_elements[y], x <= y)
      assert_eq(all_elements[x] >= all_elements[y], x >= y)
      assert_eq(all_elements[x] > all_elements[y], x > y)

# Ordering and recursion.
r = []
r.append(1)
r.append(r)
r.append(2)
assert_false(r < r)
assert_true(r <= r)
assert_true(r >= r)
assert_false(r > r)

r1 = [0]
r2 = [0, r1, 1]
r1.append(r2)
r1.append(1)
assert_false(r1 < r2)
assert_true(r1 <= r2)
assert_true(r1 >= r2)
assert_false(r1 > r2)


assert_fail('''[] < 1''')

# Weak ordering for the elements in the list.
assert_false([set()] < [set()])
assert_true([set()] <= [set()])
assert_true([set()] >= [set()])
assert_false([set()] > [set()])
assert_false([{}] < [{}])
assert_true([{}] <= [{}])
assert_true([{}] >= [{}])
assert_false([{}] > [{}])

# Weak ordering fails if elemens are not equal.
assert_fail('''[{}] < [{None: None}]''')
assert_fail('''[set()] < [set([1])]''')



