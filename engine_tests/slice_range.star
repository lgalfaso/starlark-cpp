assert_eq(range(1, 1000, 3)[2:40:3], range(7, 121, 9))
assert_fail('''
1[:]
''', error_message = """'int' object is not subscriptable
    1 | 1[:]
      | ~^^^
""")
