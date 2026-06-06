assert_fail('''
'abc'.removesuffix()
''')

assert_eq(''.removesuffix(''), '')
assert_eq('a'.removesuffix(''), 'a')
assert_eq(''.removesuffix('a'), '')
assert_eq('aba'.removesuffix('a'), 'ab')
assert_eq('aba'.removesuffix('b'), 'aba')
assert_eq('aba'.removesuffix('x'), 'aba')
assert_eq('aabaa'.removesuffix('aa'), 'aab')
assert_eq('aabaa'.removesuffix('aaa'), 'aabaa')

assert_fail('''
'abc'.removesuffix(b'')
''')


assert_fail('''
'abc'.removesuffix('', '')
''')

assert_fail('''
'abc'.removesuffix(suffix = 'a')
''')


