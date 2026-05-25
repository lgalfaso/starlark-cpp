all_entries = ['', 'A', 'a', 'ab', '∑']
def run_test():
  for i, a in enumerate(all_entries):
    for j, b in enumerate(all_entries):
      assert_eq(i < j, a < b)
      assert_eq(i <= j, a <= b)
      assert_eq(i >= j, a >= b)
      assert_eq(i > j, a > b)
