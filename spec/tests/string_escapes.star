assert_eq("\a\b\f\n\r\t\v", "\x07\x08\x0C\x0A\x0D\x09\x0B")
assert_eq("abc\
def", "abcdef")
assert_eq("\0\12\101-\132\119", "\x00\nA-Z\t9")
assert_eq("\u0041\u0414\u754c\U0001F600", "AД界😀")
assert_eq("""This is a multi-line string
That should be consistent regardless of the platform convention
it should always behave the same""", "This is a multi-line string\nThat should be consistent regardless of the platform convention\nit should always behave the same")
assert_eq(r"a\nb", "a\\nb")
assert_eq(r"a\
b", "a\\\nb")
