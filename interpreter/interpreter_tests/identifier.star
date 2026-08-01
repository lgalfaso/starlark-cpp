assert_fail('''
a = b
''', error_message = "name 'b' is not defined\n    1 | a = b\n      |     ^\n", allow_static_error = True)
