
all_elements = [
  (),
  (0,),
  (0, 1),
  (1,),
  (1, 0),
  (2,),
  (2, 0),
  (2, 1)
]

def test():
  for x in range(len(all_elements)):
    for y in range(len(all_elements)):
      assert_eq(all_elements[x] < all_elements[y], x < y)
      assert_eq(all_elements[x] <= all_elements[y], x <= y)
      assert_eq(all_elements[x] >= all_elements[y], x >= y)
      assert_eq(all_elements[x] > all_elements[y], x > y)

test()

assert_fail('''() < 1''')
assert_fail('''() < []''')

