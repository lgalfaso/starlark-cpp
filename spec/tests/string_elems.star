assert_eq(type("".elems()), "string.elems")
assert_eq(type("".elem_ords()), "string.elem_ords")
assert_eq(type("".codepoints()), "string.codepoints")
assert_eq(type("".codepoint_ords()), "string.codepoint_ords")


assert_false(not not "".elems())
assert_true(not not "a".elems())
assert_false(not not "".elem_ords())
assert_true(not not "a".elem_ords())
assert_false(not not "".codepoints())
assert_true(not not "a".codepoints())
assert_false(not not "".codepoint_ords())
assert_true(not not "a".codepoint_ords())


assert_eq(len("".elems()), 0)
assert_eq(len("a".elems()), 1)
assert_eq(len("ab".elems()), 2)
assert_eq(len("Περιπτώσεις".elems()), 11)
assert_eq(len("".elem_ords()), 0)
assert_eq(len("a".elem_ords()), 1)
assert_eq(len("ab".elem_ords()), 2)
assert_eq(len("Περιπτώσεις".elem_ords()), 11)
assert_eq(len("".codepoints()), 0)
assert_eq(len("a".codepoints()), 1)
assert_eq(len("ab".codepoints()), 2)
assert_eq(len("Περιπτώσεις".codepoints()), 11)
assert_eq(len("".codepoint_ords()), 0)
assert_eq(len("a".codepoint_ords()), 1)
assert_eq(len("ab".codepoint_ords()), 2)
assert_eq(len("Περιπτώσεις".codepoint_ords()), 11)


assert_fail('''
{"".elems(): None}
''')
assert_fail('''
{"".elem_ords(): None}
''')
assert_fail('''
{"".codepoints(): None}
''')
assert_fail('''
{"".codepoint_ords(): None}
''')


assert_eq(str("abc".elems()), '"abc".elems()')
assert_eq(str("Περιπτώσεις".elems()), '"Περιπτώσεις".elems()')
assert_eq(str("abc".elem_ords()), '"abc".elem_ords()')
assert_eq(str("Περιπτώσεις".elem_ords()), '"Περιπτώσεις".elem_ords()')
assert_eq(str("abc".codepoints()), '"abc".codepoints()')
assert_eq(str("Περιπτώσεις".codepoints()), '"Περιπτώσεις".codepoints()')
assert_eq(str("abc".codepoint_ords()), '"abc".codepoint_ords()')
assert_eq(str("Περιπτώσεις".codepoint_ords()), '"Περιπτώσεις".codepoint_ords()')


assert_eq(str("abc".elems()[1:None:None]), '"bc".elems()')
assert_eq(str("abc".elems()[None:1:None]), '"a".elems()')
assert_eq(str("abc".elems()[None:None:-1]), '"cba".elems()')
assert_eq(str("Περιπτώσεις".elems()[1:None:None]), '"εριπτώσεις".elems()')
assert_eq(str("Περιπτώσεις".elems()[None:1:None]), '"Π".elems()')
assert_eq(str("Περιπτώσεις".elems()[None:None:-1]), '"ςιεσώτπιρεΠ".elems()')

assert_eq(str("abc".elem_ords()[1:None:None]), '"bc".elem_ords()')
assert_eq(str("abc".elem_ords()[None:1:None]), '"a".elem_ords()')
assert_eq(str("abc".elem_ords()[None:None:-1]), '"cba".elem_ords()')
assert_eq(str("Περιπτώσεις".elem_ords()[1:None:None]), '"εριπτώσεις".elem_ords()')
assert_eq(str("Περιπτώσεις".elem_ords()[None:1:None]), '"Π".elem_ords()')
assert_eq(str("Περιπτώσεις".elem_ords()[None:None:-1]), '"ςιεσώτπιρεΠ".elem_ords()')

assert_eq(str("abc".codepoints()[1:None:None]), '"bc".codepoints()')
assert_eq(str("abc".codepoints()[None:1:None]), '"a".codepoints()')
assert_eq(str("abc".codepoints()[None:None:-1]), '"cba".codepoints()')
assert_eq(str("Περιπτώσεις".codepoints()[1:None:None]), '"εριπτώσεις".codepoints()')
assert_eq(str("Περιπτώσεις".codepoints()[None:1:None]), '"Π".codepoints()')
assert_eq(str("Περιπτώσεις".codepoints()[None:None:-1]), '"ςιεσώτπιρεΠ".codepoints()')

assert_eq(str("abc".codepoint_ords()[1:None:None]), '"bc".codepoint_ords()')
assert_eq(str("abc".codepoint_ords()[None:1:None]), '"a".codepoint_ords()')
assert_eq(str("abc".codepoint_ords()[None:None:-1]), '"cba".codepoint_ords()')
assert_eq(str("Περιπτώσεις".codepoint_ords()[1:None:None]), '"εριπτώσεις".codepoint_ords()')
assert_eq(str("Περιπτώσεις".codepoint_ords()[None:1:None]), '"Π".codepoint_ords()')
assert_eq(str("Περιπτώσεις".codepoint_ords()[None:None:-1]), '"ςιεσώτπιρεΠ".codepoint_ords()')


assert_fail('''
"abc".elems()[True::]
''')
assert_fail('''
"abc".elem_ords()[True::]
''')
assert_fail('''
"abc".codepoints()[True::]
''')
assert_fail('''
"abc".codepoint_ords()[True::]
''')


assert_fail('''
"abc".elems()[-4]
''')
assert_eq("abc".elems()[-3], "a")
assert_eq("abc".elems()[-2], "b")
assert_eq("abc".elems()[-1], "c")
assert_eq("abc".elems()[0], "a")
assert_eq("abc".elems()[1], "b")
assert_eq("abc".elems()[2], "c")
assert_fail('''
"abc".elems()[3]
''')

assert_fail('''
"abc".elem_ords()[-4]
''')
assert_eq("abc".elem_ords()[-3], 97)
assert_eq("abc".elem_ords()[-2], 98)
assert_eq("abc".elem_ords()[-1], 99)
assert_eq("abc".elem_ords()[0], 97)
assert_eq("abc".elem_ords()[1], 98)
assert_eq("abc".elem_ords()[2], 99)
assert_fail('''
"abc".elem_ords()[3]
''')

assert_fail('''
"abc".codepoints()[-4]
''')
assert_eq("abc".codepoints()[-3], "a")
assert_eq("abc".codepoints()[-2], "b")
assert_eq("abc".codepoints()[-1], "c")
assert_eq("abc".codepoints()[0], "a")
assert_eq("abc".codepoints()[1], "b")
assert_eq("abc".codepoints()[2], "c")
assert_fail('''
"abc".codepoints()[3]
''')

assert_fail('''
"abc".codepoint_ords()[-4]
''')
assert_eq("abc".codepoint_ords()[-3], 97)
assert_eq("abc".codepoint_ords()[-2], 98)
assert_eq("abc".codepoint_ords()[-1], 99)
assert_eq("abc".codepoint_ords()[0], 97)
assert_eq("abc".codepoint_ords()[1], 98)
assert_eq("abc".codepoint_ords()[2], 99)
assert_fail('''
"abc".codepoint_ords()[3]
''')


assert_eq("Περιπτώσεις".elems()[-3], "ε")
assert_eq("Περιπτώσεις".elems()[-2], "ι")
assert_eq("Περιπτώσεις".elems()[-1], "ς")
assert_eq("Περιπτώσεις".elems()[0], "Π")
assert_eq("Περιπτώσεις".elems()[1], "ε")
assert_eq("Περιπτώσεις".elems()[2], "ρ")

assert_eq("Περιπτώσεις".elem_ords()[-3], 949)
assert_eq("Περιπτώσεις".elem_ords()[-2], 953)
assert_eq("Περιπτώσεις".elem_ords()[-1], 962)
assert_eq("Περιπτώσεις".elem_ords()[0], 928)
assert_eq("Περιπτώσεις".elem_ords()[1], 949)
assert_eq("Περιπτώσεις".elem_ords()[2], 961)

assert_eq("Περιπτώσεις".codepoints()[-3], "ε")
assert_eq("Περιπτώσεις".codepoints()[-2], "ι")
assert_eq("Περιπτώσεις".codepoints()[-1], "ς")
assert_eq("Περιπτώσεις".codepoints()[0], "Π")
assert_eq("Περιπτώσεις".codepoints()[1], "ε")
assert_eq("Περιπτώσεις".codepoints()[2], "ρ")

assert_eq("Περιπτώσεις".codepoint_ords()[-3], 949)
assert_eq("Περιπτώσεις".codepoint_ords()[-2], 953)
assert_eq("Περιπτώσεις".codepoint_ords()[-1], 962)
assert_eq("Περιπτώσεις".codepoint_ords()[0], 928)
assert_eq("Περιπτώσεις".codepoint_ords()[1], 949)
assert_eq("Περιπτώσεις".codepoint_ords()[2], 961)

assert_fail('''
"abc".elems()[True]
''')
assert_fail('''
"abc".elem_ords()[True]
''')
assert_fail('''
"abc".codepoints()[True]
''')
assert_fail('''
"abc".codepoint_ords()[True]
''')


assert_true("".elems() == "".elems())
assert_true("a".elems() == "a".elems())
assert_false("".elems() == "a".elems())
assert_false("a".elems() == "".elems())
assert_false("a".elems() == "b".elems())
assert_false("a".elems() == True)

assert_true("".elem_ords() == "".elem_ords())
assert_true("a".elem_ords() == "a".elem_ords())
assert_false("".elem_ords() == "a".elem_ords())
assert_false("a".elem_ords() == "".elem_ords())
assert_false("a".elem_ords() == "b".elems())
assert_false("a".elem_ords() == True)

assert_true("".codepoints() == "".codepoints())
assert_true("a".codepoints() == "a".codepoints())
assert_false("".codepoints() == "a".codepoints())
assert_false("a".codepoints() == "".codepoints())
assert_false("a".codepoints() == "b".codepoints())
assert_false("a".codepoints() == True)

assert_true("".codepoint_ords() == "".codepoint_ords())
assert_true("a".codepoint_ords() == "a".codepoint_ords())
assert_false("".codepoint_ords() == "a".codepoint_ords())
assert_false("a".codepoint_ords() == "".codepoint_ords())
assert_false("a".codepoint_ords() == "b".codepoints())
assert_false("a".codepoint_ords() == True)

assert_false("".elems() == "".elem_ords())
assert_false("".elems() == "".codepoints())
assert_false("".elems() == "".codepoint_ords())
assert_false("".elem_ords() == "".codepoints())
assert_false("".elem_ords() == "".codepoint_ords())
assert_false("".codepoints() == "".codepoint_ords())


assert_false("" in "".elems())
assert_false("" in "a".elems())
assert_true("a" in "a".elems())
assert_true("a" in "ab".elems())
assert_false("ab" in "ab".elems())
assert_false("x" in "ab".elems())
assert_true("ε" in "Περιπτώσεις".elems())
assert_false("Πε" in "Περιπτώσεις".elems())

assert_false("" in "".codepoints())
assert_false("" in "a".codepoints())
assert_true("a" in "a".codepoints())
assert_true("a" in "ab".codepoints())
assert_false("ab" in "ab".codepoints())
assert_false("x" in "ab".codepoints())
assert_true("ε" in "Περιπτώσεις".codepoints())
assert_false("Πε" in "Περιπτώσεις".codepoints())
 
assert_false(97 in "".elem_ords())
assert_true(97 in "a".elem_ords())
assert_true(97 in "ab".elem_ords())
assert_true(97 in "ab".elem_ords())
assert_false(120 in "ab".elem_ords())
assert_false(120 in "ab".elem_ords())
assert_true(949 in "Περιπτώσεις".elem_ords())
assert_true(949 in "Περιπτώσεις".elem_ords())
 
assert_false(97 in "".codepoint_ords())
assert_true(97 in "a".codepoint_ords())
assert_true(97 in "ab".codepoint_ords())
assert_true(97 in "ab".codepoint_ords())
assert_false(120 in "ab".codepoint_ords())
assert_false(120 in "ab".codepoint_ords())
assert_true(949 in "Περιπτώσεις".codepoint_ords())
assert_true(949 in "Περιπτώσεις".codepoint_ords())


assert_fail('''
True in "".elems()
''')
assert_fail('''
97 in "".elems()
''')
assert_fail('''
True in "".elem_ords()
''')
assert_fail('''
'a' in "".elem_ords()
''')
assert_fail('''
True in "".codepoints()
''')
assert_fail('''
97 in "".codepoints()
''')
assert_fail('''
True in "".codepoint_ords()
''')
assert_fail('''
'a' in "".codepoint_ords()
''')


assert_eq(list("abc".elems()), ["a", "b", "c"])
assert_eq(list("abc".elem_ords()), [97, 98, 99])
assert_eq(list("abc".codepoints()), ["a", "b", "c"])
assert_eq(list("abc".codepoint_ords()), [97, 98, 99])
assert_eq(list("Περιπτώσεις".elems()), ["Π", "ε", "ρ", "ι", "π", "τ", "ώ", "σ", "ε", "ι", "ς"])
assert_eq(list("Περιπτώσεις".elem_ords()), [928, 949, 961, 953, 960, 964, 974, 963, 949, 953, 962])
assert_eq(list("Περιπτώσεις".codepoints()), ["Π", "ε", "ρ", "ι", "π", "τ", "ώ", "σ", "ε", "ι", "ς"])
assert_eq(list("Περιπτώσεις".codepoint_ords()), [928, 949, 961, 953, 960, 964, 974, 963, 949, 953, 962])

