# Octal
assert_eq(b'\000\1\37\377', b'\x00\x01\x1f\xff')
assert_fail(r'b"\400"', allow_static_error = True)

# Traditional escapes.
assert_eq(b"\a\b\f\n\r\t\v", b"\x07\x08\x0C\x0A\x0D\x09\x0B")

# Unicode.
assert_eq(b"\u0041\u0414\u754c\U0001F600", b"A\xd0\x94\xe7\x95\x8c\xf0\x9f\x98\x80")
assert_fail(r'b"\ud800"', allow_static_error = True)  # Surrogate.
assert_fail(r'b"\U00110000"', allow_static_error = True)  # Outside range.
