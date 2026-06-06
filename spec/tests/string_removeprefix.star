assert_fail('''
'abc'.removeprefix()
''')

assert_eq(''.removeprefix(''), '')
assert_eq('a'.removeprefix(''), 'a')
assert_eq(''.removeprefix('a'), '')
assert_eq('aba'.removeprefix('a'), 'ba')
assert_eq('aba'.removeprefix('b'), 'aba')
assert_eq('aba'.removeprefix('x'), 'aba')
assert_eq('aabaa'.removeprefix('aa'), 'baa')
assert_eq('aabaa'.removeprefix('aaa'), 'aabaa')

assert_fail('''
'abc'.removeprefix(b'')
''')


assert_fail('''
'abc'.removeprefix('', '')
''')

assert_fail('''
'abc'.removeprefix(prefix = 'a')
''')


