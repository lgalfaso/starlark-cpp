# Traditional escape sequences.
assert_eq("\a\b\f\n\r\t\v\\", "\x07\x08\x0C\x0A\x0D\x09\x0B\x5C")

# Escaped quotes.
assert_eq('Have you read "To Kill a Mockingbird?"', "Have you read \"To Kill a Mockingbird?\"")
assert_eq("Yes, it's a classic.", 'Yes, it\'s a classic.')

# Escaped newline.
## Non-raw string.
assert_eq("abc\
def", "abcdef")
## Raw string.
assert_eq(r"a\
b", "a\\\nb")

# Octal
assert_eq("\0\12\101-\132\119", "\x00\nA-Z\t9")
assert_fail(r'"\200"', allow_static_error = True)

# Hex
assert_eq("\x41\x42\x43", "ABC")
assert_fail(r'"\x80"', allow_static_error = True)

# Unicode.
assert_eq("\u0041\u0414\u754c\U0001F600", "AД界😀")
assert_fail(r'"\udf00"', allow_static_error = True)  # Surrogate.
assert_fail(r'"\U00110000"', allow_static_error = True)  # Outside of range.

# Raw string linefeed to carriage return conversion.
assert_eq("""This is a multi-line string
That should be consistent regardless of the platform convention
it should always behave the same""", "This is a multi-line string\nThat should be consistent regardless of the platform convention\nit should always behave the same")
assert_eq(r"a\nb", "a\\nb")
