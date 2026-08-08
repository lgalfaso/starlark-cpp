assert_fail("a = {[]: 1}", error_message = '''cannot use 'list' as a dict key (unhashable type: 'list')
    1 | a = {[]: 1}
      |     ^^^^^^^
''')

