assert_fail("""
while True:
  break
""", allow_static_error = True)
