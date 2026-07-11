assert_fail('''
a = b
''', error_message = "name 'b' is not defined", allow_static_error = True)
