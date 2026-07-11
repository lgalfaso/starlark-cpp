assert_fail("""
x = max(3, 4, 6),
""", allow_static_error = True)
assert_succeed("""
x = (max(3, 4, 6),)
""")
