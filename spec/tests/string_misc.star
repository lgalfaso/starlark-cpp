def check(value, expected):
  assert_eq((value.isalnum(), value.isalpha(), value.isdigit(), value.isspace(), value.islower(), value.isupper(), value.istitle()), expected)

check('', (False, False, False, False, False, False, False))
check(' ', (False, False, False, True, False, False, False))
check('abc', (True, True, False, False, True, False, False))
check('123', (True, False, True, False, False, False, False))
check('abc123', (True, False, False, False, True, False, False))
check('abc123!@#', (False, False, False, False, True, False, False))
check('LettersOnly', (True, True, False, False, False, False, False))
check('Letters and spaces', (False, False, False, False, False, False, False))
check('Letters And Spaces', (False, False, False, False, False, False, True))
check('LETTERS AND SPACES', (False, False, False, False, False, True, False))
check('µ', (True, True, False, False, True, False, False))
check('¼', (True, False, False, False, False, False, False))
check('\u3405', (True, True, False, False, False, False, False))

assert_fail('''
''.isalnum('')
''')
assert_fail('''
''.isalpha('')
''')
assert_fail('''
''.isdigit('')
''')
assert_fail('''
''.isspace('')
''')
assert_fail('''
''.islower('')
''')
assert_fail('''
''.isupper('')
''')
assert_fail('''
''.istitle('')
''')


def setcase(value, expected):
  assert_eq((value.lower(), value.upper(), value.title()), expected)

setcase('', ('', '', ''))
setcase('a', ('a', 'A', 'A'))
setcase('A', ('a', 'A', 'A'))
setcase('abc', ('abc', 'ABC', 'Abc'))
setcase('aeiou', ('aeiou', 'AEIOU', 'Aeiou'))
setcase('abc1234', ('abc1234', 'ABC1234', 'Abc1234'))
setcase('heLlo woRld!', ('hello world!', 'HELLO WORLD!', 'Hello World!'))
setcase('\u0390', ('\u0390', '\u0399\u0308\u0301', '\u0399\u0308\u0301'))
setcase('ΠΕΡΙΠΤΏΣΕΙΣ', ('περιπτώσεις', 'ΠΕΡΙΠΤΏΣΕΙΣ', 'Περιπτώσεις'))
setcase('Σ Σ', ('σ σ', 'Σ Σ', 'Σ Σ'))
setcase(' Σ Σ ', (' σ σ ', ' Σ Σ ', ' Σ Σ '))
setcase('Σ.Σ.Σ', ('σ.σ.ς', 'Σ.Σ.Σ', 'Σ.σ.ς'))
setcase(' Σ.Σ.Σ ', (' σ.σ.ς ' , ' Σ.Σ.Σ ', ' Σ.σ.ς '))


