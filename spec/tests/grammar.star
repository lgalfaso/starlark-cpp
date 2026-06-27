# No iteration at top level.
assert_fail('''
for x in []:
  pass
''')

# Single assignment at top level.
assert_fail('''
a = 1
a = 2
''')

assert_succeed('''\uFEFF
# A program can start with a BOM character.
a = 1
''')

