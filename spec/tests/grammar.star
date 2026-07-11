# No iteration at top level.
assert_fail('''
for x in []:
  pass
''', allow_static_error = True)

# Single assignment at top level.
assert_fail('''
a = 1
a = 2
''', allow_static_error = True)

assert_succeed('''\uFEFF
# A program can start with a BOM character.
a = 1
''')

