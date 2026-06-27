assert_eq(''.splitlines(), [])
assert_eq(' '.splitlines(), [' '])
assert_eq('abc'.splitlines(), ['abc'])
assert_eq('ab\nc'.splitlines(), ['ab', 'c'])
assert_eq('\n\n'.splitlines(), ['', ''])
assert_eq('abc\r\ndef\r\n'.splitlines(), ['abc', 'def'])
assert_eq('abc\ndef\rghi\r\njkl\vmno\fpqr\x1cstu\x1dvwx\x1eyza\u0085bcd\u2028efg\u2029hij'.splitlines(),
       ['abc', 'def', 'ghi', 'jkl', 'mno', 'pqr', 'stu', 'vwx', 'yza', 'bcd', 'efg', 'hij'])

assert_eq(''.splitlines(True), [])
assert_eq(''.splitlines(False), [])
assert_eq(' '.splitlines(True), [' '])
assert_eq(' '.splitlines(False), [' '])
assert_eq('abc'.splitlines(True), ['abc'])
assert_eq('abc'.splitlines(False), ['abc'])
assert_eq('ab\nc'.splitlines(True), ['ab\n', 'c'])
assert_eq('ab\nc'.splitlines(False), ['ab', 'c'])
assert_eq('\n\n'.splitlines(True), ['\n', '\n'])
assert_eq('\n\n'.splitlines(False), ['', ''])
assert_eq('abc\r\ndef\r\n'.splitlines(True), ['abc\r\n', 'def\r\n'])
assert_eq('abc\r\ndef\r\n'.splitlines(False), ['abc', 'def'])

assert_fail('''
'abc'.splitlines(1)
''')

assert_fail('''
'abc'.splitlines(True, True)
''')

assert_eq('abc'.splitlines(keepends = True), ['abc'])
