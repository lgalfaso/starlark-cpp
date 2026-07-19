
# Trailing comma in for statement
assert_fail('''
def test():
  d = {}
  for k, v, in d.items(): pass
''', allow_static_error = True)

# Trailing comma in for closure.
assert_fail('''_ = [(v, k) for k, v, in {}.items()]''', allow_static_error = True)

# Trailing comma in function call
max(3, 1, 4, 1,)
# In list
[1, 2, 3, ]
# In dictionary.
{1: 2, 3: 4, }


