assert_eq(''.split(), [])
assert_eq('  '.split(), [])
assert_eq('abc'.split(), ['abc'])
assert_eq('  abc  '.split(), ['abc'])
assert_eq('  a  b  c  '.split(), ['a', 'b', 'c'])

assert_eq(''.rsplit(), [])
assert_eq('  '.rsplit(), [])
assert_eq('abc'.rsplit(), ['abc'])
assert_eq('  abc  '.rsplit(), ['abc'])
assert_eq('  a  b  c  '.rsplit(), ['a', 'b', 'c'])


assert_eq(''.split(None), [])
assert_eq('  '.split(None), [])
assert_eq('abc'.split(None), ['abc'])
assert_eq('  abc  '.split(None), ['abc'])
assert_eq('  a  b  c  '.split(None), ['a', 'b', 'c'])
assert_eq('\u2002\u2002a\u2002\u2002b\u2002\u2002c\u2002\u2002'.split(None), ['a', 'b', 'c'])

assert_eq(''.rsplit(None), [])
assert_eq('  '.rsplit(None), [])
assert_eq('abc'.rsplit(None), ['abc'])
assert_eq('  abc  '.rsplit(None), ['abc'])
assert_eq('  a  b  c  '.rsplit(None), ['a', 'b', 'c'])
assert_eq('\u2002\u2002a\u2002\u2002b\u2002\u2002c\u2002\u2002'.rsplit(None), ['a', 'b', 'c'])


assert_fail('''
'abc'.split(True)
''')
assert_fail('''
'abc'.rsplit(True)
''')
assert_fail('''
'abc'.split('')
''')
assert_fail('''
'abc'.rsplit('')
''')


assert_eq(''.split('x'), [''])
assert_eq('  '.split('x'), ['  '])
assert_eq('axbxc'.split('x'), ['a', 'b', 'c'])
assert_eq('xaxbxcx'.split('x'), ['', 'a', 'b', 'c', ''])
assert_eq('xxaxxbxxcxx'.split('x'), ['', '', 'a', '', 'b', '', 'c', '', ''])
assert_eq('xxaxxbxxcxx'.split('xx'), ['', 'a', 'b', 'c', ''])
assert_eq('xaxbxcx'.split('xx'), ['xaxbxcx'])

assert_eq(''.rsplit('x'), [''])
assert_eq('  '.rsplit('x'), ['  '])
assert_eq('axbxc'.rsplit('x'), ['a', 'b', 'c'])
assert_eq('xaxbxcx'.rsplit('x'), ['', 'a', 'b', 'c', ''])
assert_eq('xxaxxbxxcxx'.rsplit('x'), ['', '', 'a', '', 'b', '', 'c', '', ''])
assert_eq('xxaxxbxxcxx'.rsplit('xx'), ['', 'a', 'b', 'c', ''])
assert_eq('xaxbxcx'.rsplit('xx'), ['xaxbxcx'])


assert_eq(''.split('x', -1), [''])
assert_eq(''.split('x', 0), [''])
assert_eq(''.split('x', 1), [''])
assert_eq('  '.split('x', -1), ['  '])
assert_eq('  '.split('x', 0), ['  '])
assert_eq('  '.split('x', 1), ['  '])
assert_eq('axbxc'.split('x', -1), ['a', 'b', 'c'])
assert_eq('axbxc'.split('x', 0), ['axbxc'])
assert_eq('axbxc'.split('x', 1), ['a', 'bxc'])
assert_eq('xaxbxcx'.split('x', -1), ['', 'a', 'b', 'c', ''])
assert_eq('xaxbxcx'.split('x', 0), ['xaxbxcx'])
assert_eq('xaxbxcx'.split('x', 1), ['', 'axbxcx'])
assert_eq('xxaxxbxxcxx'.split('x', -1), ['', '', 'a', '', 'b', '', 'c', '', ''])
assert_eq('xxaxxbxxcxx'.split('x', 0), ['xxaxxbxxcxx'])
assert_eq('xxaxxbxxcxx'.split('x', 1), ['', 'xaxxbxxcxx'])
assert_eq('xxaxxbxxcxx'.split('xx', -1), ['', 'a', 'b', 'c', ''])
assert_eq('xxaxxbxxcxx'.split('xx', 0), ['xxaxxbxxcxx'])
assert_eq('xxaxxbxxcxx'.split('xx', 1), ['', 'axxbxxcxx'])
assert_eq('xaxbxcx'.split('xx', -1), ['xaxbxcx'])
assert_eq('xaxbxcx'.split('xx', 0), ['xaxbxcx'])
assert_eq('xaxbxcx'.split('xx', 1), ['xaxbxcx'])

assert_eq(''.rsplit('x', -1), [''])
assert_eq(''.rsplit('x', 0), [''])
assert_eq(''.rsplit('x', 1), [''])
assert_eq('  '.rsplit('x', -1), ['  '])
assert_eq('  '.rsplit('x', 0), ['  '])
assert_eq('  '.rsplit('x', 1), ['  '])
assert_eq('axbxc'.rsplit('x', -1), ['a', 'b', 'c'])
assert_eq('axbxc'.rsplit('x', 0), ['axbxc'])
assert_eq('axbxc'.rsplit('x', 1), ['axb', 'c'])
assert_eq('xaxbxcx'.rsplit('x', -1), ['', 'a', 'b', 'c', ''])
assert_eq('xaxbxcx'.rsplit('x', 0), ['xaxbxcx'])
assert_eq('xaxbxcx'.rsplit('x', 1), ['xaxbxc', ''])
assert_eq('xxaxxbxxcxx'.rsplit('x', -1), ['', '', 'a', '', 'b', '', 'c', '', ''])
assert_eq('xxaxxbxxcxx'.rsplit('x', 0), ['xxaxxbxxcxx'])
assert_eq('xxaxxbxxcxx'.rsplit('x', 1), ['xxaxxbxxcx', ''])
assert_eq('xxaxxbxxcxx'.rsplit('xx', -1), ['', 'a', 'b', 'c', ''])
assert_eq('xxaxxbxxcxx'.rsplit('xx', 0), ['xxaxxbxxcxx'])
assert_eq('xxaxxbxxcxx'.rsplit('xx', 1), ['xxaxxbxxc', ''])
assert_eq('xaxbxcx'.rsplit('xx', -1), ['xaxbxcx'])
assert_eq('xaxbxcx'.rsplit('xx', 0), ['xaxbxcx'])
assert_eq('xaxbxcx'.rsplit('xx', 1), ['xaxbxcx'])


assert_fail('''
'abc'.split('', 0)
''')
assert_fail('''
'abc'.rsplit('', 0)
''')

assert_fail('''
'abc'.split(True, 0)
''')
assert_fail('''
'abc'.rsplit(True, 0)
''')

assert_fail('''
'abc'.split(None, True)
''')
assert_fail('''
'abc'.rsplit(None, True)
''')


assert_eq(''.split(None, -1), [])
assert_eq(''.split(None, 0), [])
assert_eq(''.split(None, 1), [])
assert_eq('  '.split(None, -1), [])
assert_eq('  '.split(None, 0), [])
assert_eq('  '.split(None, 1), [])
assert_eq('abc'.split(None, -1), ['abc'])
assert_eq('abc'.split(None, 0), ['abc'])
assert_eq('abc'.split(None, 1), ['abc'])
assert_eq('  abc  '.split(None, -1), ['abc'])
assert_eq('  abc  '.split(None, 0), ['abc  '])
assert_eq('  abc  '.split(None, 1), ['abc'])
assert_eq('  a  b  c  '.split(None, -1), ['a', 'b', 'c'])
assert_eq('  a  b  c  '.split(None, 0), ['a  b  c  '])
assert_eq('  a  b  c  '.split(None, 1), ['a', 'b  c  '])

assert_eq(''.rsplit(None, -1), [])
assert_eq(''.rsplit(None, 0), [])
assert_eq(''.rsplit(None, 1), [])
assert_eq('  '.rsplit(None, -1), [])
assert_eq('  '.rsplit(None, 0), [])
assert_eq('  '.rsplit(None, 1), [])
assert_eq('abc'.rsplit(None, -1), ['abc'])
assert_eq('abc'.rsplit(None, 0), ['abc'])
assert_eq('abc'.rsplit(None, 1), ['abc'])
assert_eq('  abc  '.rsplit(None, -1), ['abc'])
assert_eq('  abc  '.rsplit(None, 0), ['  abc'])
assert_eq('  abc  '.rsplit(None, 1), ['abc'])
assert_eq('  a  b  c  '.rsplit(None, -1), ['a', 'b', 'c'])
assert_eq('  a  b  c  '.rsplit(None, 0), ['  a  b  c'])
assert_eq('  a  b  c  '.rsplit(None, 1), ['  a  b', 'c'])


assert_fail('''
'abc'.split(None, True)
''')
assert_fail('''
'abc'.rsplit(None, True)
''')

assert_fail('''
'abc'.split(None, 0, 1)
''')
assert_fail('''
'abc'.rsplit(None, 0, 1)
''')

assert_eq('abc'.split('b', maxsplit=-1), ['a', 'c'])
assert_eq('abc'.rsplit('b', maxsplit=-1), ['a', 'c'])

