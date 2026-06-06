assert_fail('''
'abc'.partition()
''')
assert_fail('''
'abc'.rpartition()
''')

assert_fail('''
'abc'.partition('')
''')
assert_fail('''
'abc'.rpartition('')
''')

assert_eq('abc'.partition('banana'), ('abc', '', ''))
assert_eq('abc'.partition('a'), ('', 'a', 'bc'))
assert_eq('abc'.partition('b'), ('a', 'b', 'c'))
assert_eq('abc'.partition('c'), ('ab', 'c', ''))
assert_eq('abcabc'.partition('a'), ('', 'a', 'bcabc'))
assert_eq('abcabc'.partition('b'), ('a', 'b', 'cabc'))
assert_eq('abcabc'.partition('c'), ('ab', 'c', 'abc'))

assert_eq('abc'.rpartition('banana'), ('', '', 'abc'))
assert_eq('abc'.rpartition('a'), ('', 'a', 'bc'))
assert_eq('abc'.rpartition('b'), ('a', 'b', 'c'))
assert_eq('abc'.rpartition('c'), ('ab', 'c', ''))
assert_eq('abcabc'.rpartition('a'), ('abc', 'a', 'bc'))
assert_eq('abcabc'.rpartition('b'), ('abca', 'b', 'c'))
assert_eq('abcabc'.rpartition('c'), ('abcab', 'c', ''))

assert_fail('''
'abc'.partition(sep = 'a')
''')
assert_fail('''
'abc'.rpartition(sep = 'a')
''')

assert_fail('''
'abc'.partition(True)
''')
assert_fail('''
'abc'.rpartition(True)
''')

