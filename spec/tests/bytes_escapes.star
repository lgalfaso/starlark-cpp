assert_eq(b'\000\377', b'\x00\xff')
assert_eq(b"\a\b\f\n\r\t\v", b"\x07\x08\x0C\x0A\x0D\x09\x0B")
assert_eq(b"\u0041\u0414\u754c\U0001F600", b"A\xd0\x94\xe7\x95\x8c\xf0\x9f\x98\x80")
