assert_fail('''
[1] is [1]
''', allow_static_error = True)

assert_fail('''
1 < 2 < 3
''', allow_static_error = True)

assert_fail('''
1 in [1] not in [2]
''', allow_static_error = True)
