## def rr(a, b, c):
##     return 'test({}, {}, {}, "{}", "{}", "{}", "{}", "{}", "{}")'.format(a, b, c, *[str('abcde'[:x][a:b:c]) for x in range(6)])
## print("\n".join([rr(a,b,c) for a in (None, -1, 0, 1) for b in (None, -1, 0, 1) for c in (None, -1, 1)]))

def test(start, end, stride, v0, v1, v2, v3, v4, v5):
  assert_eq(""[start:end:stride], v0)
  assert_eq("a"[start:end:stride], v1)
  assert_eq("ab"[start:end:stride], v2)
  assert_eq("abc"[start:end:stride], v3)
  assert_eq("abcd"[start:end:stride], v4)
  assert_eq("abcde"[start:end:stride], v5)

test(None, None, None, "", "a", "ab", "abc", "abcd", "abcde")
test(None, None, -1, "", "a", "ba", "cba", "dcba", "edcba")
test(None, None, 1, "", "a", "ab", "abc", "abcd", "abcde")
test(None, -1, None, "", "", "a", "ab", "abc", "abcd")
test(None, -1, -1, "", "", "", "", "", "")
test(None, -1, 1, "", "", "a", "ab", "abc", "abcd")
test(None, 0, None, "", "", "", "", "", "")
test(None, 0, -1, "", "", "b", "cb", "dcb", "edcb")
test(None, 0, 1, "", "", "", "", "", "")
test(None, 1, None, "", "a", "a", "a", "a", "a")
test(None, 1, -1, "", "", "", "c", "dc", "edc")
test(None, 1, 1, "", "a", "a", "a", "a", "a")
test(-1, None, None, "", "a", "b", "c", "d", "e")
test(-1, None, -1, "", "a", "ba", "cba", "dcba", "edcba")
test(-1, None, 1, "", "a", "b", "c", "d", "e")
test(-1, -1, None, "", "", "", "", "", "")
test(-1, -1, -1, "", "", "", "", "", "")
test(-1, -1, 1, "", "", "", "", "", "")
test(-1, 0, None, "", "", "", "", "", "")
test(-1, 0, -1, "", "", "b", "cb", "dcb", "edcb")
test(-1, 0, 1, "", "", "", "", "", "")
test(-1, 1, None, "", "a", "", "", "", "")
test(-1, 1, -1, "", "", "", "c", "dc", "edc")
test(-1, 1, 1, "", "a", "", "", "", "")
test(0, None, None, "", "a", "ab", "abc", "abcd", "abcde")
test(0, None, -1, "", "a", "a", "a", "a", "a")
test(0, None, 1, "", "a", "ab", "abc", "abcd", "abcde")
test(0, -1, None, "", "", "a", "ab", "abc", "abcd")
test(0, -1, -1, "", "", "", "", "", "")
test(0, -1, 1, "", "", "a", "ab", "abc", "abcd")
test(0, 0, None, "", "", "", "", "", "")
test(0, 0, -1, "", "", "", "", "", "")
test(0, 0, 1, "", "", "", "", "", "")
test(0, 1, None, "", "a", "a", "a", "a", "a")
test(0, 1, -1, "", "", "", "", "", "")
test(0, 1, 1, "", "a", "a", "a", "a", "a")
test(1, None, None, "", "", "b", "bc", "bcd", "bcde")
test(1, None, -1, "", "a", "ba", "ba", "ba", "ba")
test(1, None, 1, "", "", "b", "bc", "bcd", "bcde")
test(1, -1, None, "", "", "", "b", "bc", "bcd")
test(1, -1, -1, "", "", "", "", "", "")
test(1, -1, 1, "", "", "", "b", "bc", "bcd")
test(1, 0, None, "", "", "", "", "", "")
test(1, 0, -1, "", "", "b", "b", "b", "b")
test(1, 0, 1, "", "", "", "", "", "")
test(1, 1, None, "", "", "", "", "", "")
test(1, 1, -1, "", "", "", "", "", "")
test(1, 1, 1, "", "", "", "", "", "")

def test_unicode():
  unicode_value = ("Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις " +
                  "Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις " +
                  "Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις " +
                  "Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις " +
                  "Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις " +
                  "Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις " +
                  "Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις " +
                  "Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις " +
                  "Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις " +
                  "Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις " +
                  "Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις " +
                  "Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις Περιπτώσεις ")

  for i in range(720, 0, 12):
    assert_eq(unicode_value[i:(i + 12): None], "Περιπτώσεις ")

  assert_eq(unicode_value[0:None:12], "ΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠΠ")
  assert_eq(unicode_value[1:None:12], "εεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεε")
  assert_eq(unicode_value[2:None:12], "ρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρρ")
  assert_eq(unicode_value[3:None:12], "ιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιι")
  assert_eq(unicode_value[4:None:12], "ππππππππππππππππππππππππππππππππππππππππππππππππππππππππππππ")
  assert_eq(unicode_value[5:None:12], "ττττττττττττττττττττττττττττττττττττττττττττττττττττττττττττ")
  assert_eq(unicode_value[6:None:12], "ώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώώ")
  assert_eq(unicode_value[7:None:12], "σσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσσ")
  assert_eq(unicode_value[8:None:12], "εεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεεε")
  assert_eq(unicode_value[9:None:12], "ιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιιι")
  assert_eq(unicode_value[10:None:12],  "ςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςςς")
  assert_eq(unicode_value[11:None:12],  "                                                            ")

  value = "Περιπτώσεις";
  assert_eq(value[None:None:-5], "ςτΠ")
  assert_eq(value[None:None:-4], "ςώρ")
  assert_eq(value[None:None:-3], "ςσπε")
  assert_eq(value[None:None:-2], "ςεώπρΠ")
  assert_eq(value[None:None:-1], "ςιεσώτπιρεΠ")
  assert_eq(value[None:None:1], "Περιπτώσεις")
  assert_eq(value[None:None:2], "Πρπώες")
  assert_eq(value[None:None:3], "Πιώι")
  assert_eq(value[None:None:4], "Ππε")
  assert_eq(value[None:None:5], "Πτς")

test_unicode()

assert_fail(""""abcdef"[True:None:None]""")
assert_fail(""""abcdef"[None:True:None]""")
assert_fail(""""abcdef"[None:None:True]""")
assert_fail(""""abcdef"[None:None:0]""")

