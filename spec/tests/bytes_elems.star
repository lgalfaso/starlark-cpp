# Truthyness.
assert_false(True if b''.elems() else False)
assert_true(True if b'abc'.elems() else False)
assert_false(True if b''.elem_ords() else False)
assert_true(True if b'abc'.elem_ords() else False)

# Len
assert_eq(len(b''.elems()), 0)
assert_eq(len(b'a'.elems()), 1)
assert_eq(len(b'ab'.elems()), 2)
assert_eq(len(b''.elem_ords()), 0)
assert_eq(len(b'a'.elem_ords()), 1)
assert_eq(len(b'ab'.elem_ords()), 2)

# Hashable
assert_fail('''{b''.elems(): None}''')
assert_fail('''{b''.elem_ords(): None}''')

# Str
assert_eq(str(b'abc'.elems()), "b\"abc\".elems()")
assert_eq(str(b'abc'.elem_ords()), "b\"abc\".elem_ords()")

# Slice
assert_eq(str(b'abc'.elems()[1:]), 'b"bc".elems()')
assert_eq(str(b'abc'.elems()[:1]), 'b"a".elems()')
assert_eq(str(b'abc'.elems()[::-1]), 'b"cba".elems()')
assert_eq(str(b'abc'.elem_ords()[1:]), 'b"bc".elem_ords()')
assert_eq(str(b'abc'.elem_ords()[:1]), 'b"a".elem_ords()')
assert_eq(str(b'abc'.elem_ords()[::-1]), 'b"cba".elem_ords()')
assert_fail('''b'abc'.elems()[True:]''')
assert_fail('''b'abc'.elem_ords()[True:]''')

# Index
assert_fail('''b'abc'.elems()[-4]''')
assert_eq(b'abc'.elems()[-3], b'a')
assert_eq(b'abc'.elems()[-2], b'b')
assert_eq(b'abc'.elems()[-1], b'c')
assert_eq(b'abc'.elems()[0], b'a')
assert_eq(b'abc'.elems()[1], b'b')
assert_eq(b'abc'.elems()[2], b'c')
assert_fail('''b'abc'.elems()[3]''')

assert_fail('''b'abc'.elem_ords()[-4]''')
assert_eq(b'abc'.elem_ords()[-3], 97)
assert_eq(b'abc'.elem_ords()[-2], 98)
assert_eq(b'abc'.elem_ords()[-1], 99)
assert_eq(b'abc'.elem_ords()[0], 97)
assert_eq(b'abc'.elem_ords()[1], 98)
assert_eq(b'abc'.elem_ords()[2], 99)
assert_fail('''b'abc'.elem_ords()[3]''')

assert_fail('''b'abc'.elems()[False]''')
assert_fail('''b'abc'.elem_ords()[False]''')

# Equals
assert_true(b''.elems() == b''.elems())
assert_true(b'a'.elems() == b'a'.elems())
assert_false(b''.elems() == b'a'.elems())
assert_false(b'a'.elems() == b''.elems())
assert_false(b'a'.elems() == b'b'.elems())
assert_false(b''.elems() == False)

assert_true(b''.elem_ords() == b''.elem_ords())
assert_true(b'a'.elem_ords() == b'a'.elem_ords())
assert_false(b''.elem_ords() == b'a'.elem_ords())
assert_false(b'a'.elem_ords() == b''.elem_ords())
assert_false(b'a'.elem_ords() == b'b'.elem_ords())
assert_false(b''.elem_ords() == False)

assert_false(b''.elems() == b''.elem_ords())


# Operator `in`
assert_false(b'' in b''.elems())
assert_false(b'' in b'a'.elems())
assert_true(b'a' in b'a'.elems())
assert_true(b'a' in b'abc'.elems())
assert_true(97 in b'a'.elems())
assert_false(b'x' in b'abc'.elems())
assert_fail('''False in b''.elems()''')

assert_false(b'' in b''.elem_ords())
assert_false(b'' in b'a'.elem_ords())
assert_true(b'a' in b'a'.elem_ords())
assert_true(b'a' in b'abc'.elem_ords())
assert_true(97 in b'a'.elem_ords())
assert_false(b'x' in b'abc'.elem_ords())
assert_fail('''False in b''.elem_ords()''')


# Iterate
assert_eq([x for x in b'abc'.elems()], [b'a', b'b', b'c'])
assert_eq([x for x in b'abc'.elem_ords()], [97, 98, 99])

