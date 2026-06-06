assert_fail('''
''.join()
''')

assert_fail('''
''.join('')
''')

assert_fail('''
''.join(True)
''')

assert_eq('abc'.join(()), '')
assert_eq('abc'.join(('xyz',)), 'xyz')
assert_eq('abc'.join(('12', '34')), '12abc34')
assert_eq('abc'.join(('12', '34', '56')), '12abc34abc56')

assert_fail('''
''.join('abc', 'def')
''')

assert_fail('''
''.join(iterable = [])
''')

