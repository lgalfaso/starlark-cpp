assert_fail("""
x = max(3, 4, 6),
""")
assert_succeed("""
x = (max(3, 4, 6),)
""")
