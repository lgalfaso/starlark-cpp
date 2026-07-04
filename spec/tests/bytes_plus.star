assert_eq(b'' + b'', b'')
assert_eq(b'abc' + b'', b'abc')
assert_eq(b'' + b'def', b'def')
assert_eq(b'abc' + b'def', b'abcdef')
assert_succeed('''
a = b'abc' + b'def'
''')
assert_succeed('''
a = (b'abc' +
     b'def')
''')
assert_fail('''
a = b'abc' +
     'def'
''')
assert_fail('''
b'abc' + ''
''')


