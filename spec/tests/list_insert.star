list = [0, 1, 2, 3]
list.insert(0, "abc")
assert_eq(list, ["abc", 0, 1, 2, 3])
list.insert(2, "def")
assert_eq(list, ["abc", 0, "def", 1, 2, 3])
list.insert(-1, "ghi")
assert_eq(list, ["abc", 0, "def", 1, 2, "ghi", 3])
list.insert(-30, "jkl")
assert_eq(list, ["jkl", "abc", 0, "def", 1, 2, "ghi", 3])
list.insert(30, "mno")
assert_eq(list, ["jkl", "abc", 0, "def", 1, 2, "ghi", 3, "mno"])
list.insert(len(list) + 1, "pqr")
assert_eq(list, ["jkl", "abc", 0, "def", 1, 2, "ghi", 3, "mno", "pqr"])
list.insert(1<<64, "stu")
assert_eq(list, ["jkl", "abc", 0, "def", 1, 2, "ghi", 3, "mno", "pqr", "stu"])
list.insert(1<<64, "vwx")
assert_eq(list, ["jkl", "abc", 0, "def", 1, 2, "ghi", 3, "mno", "pqr", "stu", "vwx"])
list.insert(-1<<64, "yza")
assert_eq(list, ["yza", "jkl", "abc", 0, "def", 1, 2, "ghi", 3, "mno", "pqr", "stu", "vwx"])

assert_fail('''
def foo():
  a = [0, 1]
  for x in a:
    a.insert(0, "x")

foo()
''')

assert_fail('''
[].insert()
''')
assert_fail('''
[].insert(0)
''')
assert_fail('''
[].insert(0, 0, None)
''')
assert_fail('''
[].insert(True, 0)
''')
assert_fail('''
[].insert(0, value = "")
''')
assert_fail('''
[].insert(index = 0, value = "")
''')
