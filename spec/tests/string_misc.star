def check(value, expected):
  assert_eq((value.isalnum(), value.isalpha(), value.isdigit(), value.isspace()), expected)

check('', (False, False, False, False))
check(' ', (False, False, False, True))
check('abc', (True, True, False, False))
check('123', (True, False, True, False))
check('abc123', (True, False, False, False))
check('abc123!@#', (False, False, False, False))
check('LettersOnly', (True, True, False, False))
check('Letters and spaces', (False, False, False, False))
check('µ', (True, True, False, False))
check('¼', (True, False, False, False))
check('\u3405', (True, True, False, False))

