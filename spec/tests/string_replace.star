assert_fail('''
'abc'.replace()
''')

assert_fail('''
'abc'.replace()
''')

assert_eq(''.replace('', ''), '')
assert_eq(''.replace('', 'x'), 'x')
assert_eq('a'.replace('', 'x'), 'xax')
assert_eq('ab'.replace('', 'x'), 'xaxbx')

assert_eq(''.replace('y', ''), '')
assert_eq(''.replace('y', 'x'), '')
assert_eq('a'.replace('y', 'x'), 'a')
assert_eq('ab'.replace('y', 'x'), 'ab')
assert_eq('ab'.replace('y', 'x'), 'ab')
assert_eq('ayb'.replace('y', 'x'), 'axb')
assert_eq('ayyb'.replace('y', 'x'), 'axxb')
assert_eq('ayyyb'.replace('yy', 'x'), 'axyb')
assert_eq('ayxyb'.replace('y', 'x'), 'axxxb')
assert_eq('ayxyb'.replace('y', 'yy'), 'ayyxyyb')

assert_fail('''
'abc'.replace(True, '')
''')

assert_fail('''
'abc'.replace('', True)
''')


assert_eq(''.replace('', '', -1), '')
assert_eq(''.replace('', '', 0), '')
assert_eq(''.replace('', '', 1), '')
assert_eq(''.replace('', 'x', -1), 'x')
assert_eq(''.replace('', 'x', 0), '')
assert_eq(''.replace('', 'x', 1), 'x')
assert_eq('a'.replace('', 'x', -1), 'xax')
assert_eq('a'.replace('', 'x', 0), 'a')
assert_eq('a'.replace('', 'x', 1), 'xa')
assert_eq('ab'.replace('', 'x', -1), 'xaxbx')
assert_eq('ab'.replace('', 'x', 0), 'ab')
assert_eq('ab'.replace('', 'x', 1), 'xab')

assert_eq(''.replace('y', '', -1), '')
assert_eq(''.replace('y', '', 0), '')
assert_eq(''.replace('y', '', 1), '')
assert_eq(''.replace('y', 'x', -1), '')
assert_eq(''.replace('y', 'x', 0), '')
assert_eq(''.replace('y', 'x', 1), '')
assert_eq('a'.replace('y', 'x', -1), 'a')
assert_eq('a'.replace('y', 'x', 0), 'a')
assert_eq('a'.replace('y', 'x', 1), 'a')
assert_eq('ab'.replace('y', 'x', -1), 'ab')
assert_eq('ab'.replace( 'y', 'x', 0), 'ab')
assert_eq('ab'.replace('y', 'x', 1), 'ab')
assert_eq('ayb'.replace('y', 'x', -1), 'axb')
assert_eq('ayb'.replace('y', 'x', 0), 'ayb')
assert_eq('ayb'.replace('y', 'x', 1), 'axb')
assert_eq('ayyb'.replace('y', 'x', -1), 'axxb')
assert_eq('ayyb'.replace('y', 'x', 0), 'ayyb')
assert_eq('ayyb'.replace('y', 'x', 1), 'axyb')
assert_eq('ayxyb'.replace('y', 'x', -1), 'axxxb')
assert_eq('ayxyb'.replace('y', 'x', 0), 'ayxyb')
assert_eq('ayxyb'.replace('y', 'x', 1), 'axxyb')
assert_eq('ayxyb'.replace('y', 'yy', -1), 'ayyxyyb')
assert_eq('ayxyb'.replace('y', 'yy', 0), 'ayxyb')
assert_eq('ayxyb'.replace('y', 'yy', 1), 'ayyxyb')

assert_eq('abc'.replace('b', 'x', 327941723984723847123984712398471239847123), 'axc')
assert_eq('abc'.replace('b', 'x', -327941723984723847123984712398471239847123), 'axc')

assert_fail('''
'abc'.replace('', '', True)
''')

assert_fail('''
'abc'.replace('', '', 0, 0)
''')

assert_fail('''
'abc'.replace('', '', count = 0)
''')

