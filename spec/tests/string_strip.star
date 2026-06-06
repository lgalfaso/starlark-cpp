def check_no_arguments(value, expected):
  assert_eq((value.strip(), value.rstrip(), value.lstrip()), expected)

check_no_arguments("", ("", "", ""))
check_no_arguments(" ", ("", "", ""))
check_no_arguments(" \t", ("", "", ""))
check_no_arguments("  abc  ", ("abc", "  abc", "abc  "))
check_no_arguments("   abcdefghij  ", ("abcdefghij", "   abcdefghij", "abcdefghij  "))
check_no_arguments("  abcdefghij   ", ("abcdefghij", "  abcdefghij", "abcdefghij   "))
check_no_arguments("abcdefghij", ("abcdefghij", "abcdefghij", "abcdefghij"))
check_no_arguments("\u202Fabcdefghij\u202F", ("abcdefghij", "\u202Fabcdefghij", "abcdefghij\u202F"))


def check_with_cutset(value, cutset, expected):
  assert_eq((value.strip(cutset), value.rstrip(cutset), value.lstrip(cutset)), expected)


check_with_cutset("", "", ("", "", ""))
check_with_cutset(" ", "", (" ", " ", " "))
check_with_cutset(" \t ", " ", ("\t", " \t", "\t "))
check_with_cutset("  abc  ", "x", ("  abc  ", "  abc  ", "  abc  "))
check_with_cutset("xxabcxx", "x", ("abc", "xxabc", "abcxx"))
check_with_cutset("zyxabcdefghijzyx", "xyz", ("abcdefghij", "zyxabcdefghij", "abcdefghijzyx"))
check_with_cutset("abcdefghij", "xyz", ("abcdefghij", "abcdefghij", "abcdefghij"))
check_with_cutset("zzyyxx", "xyz", ("", "", ""))
check_with_cutset("\uFFFDabc\uFFFD", "\uFFFD", ("abc", "\uFFFDabc", "abc\uFFFD"))

assert_fail('''
"abc".strip(True)
''')

assert_fail('''
"abc".rstrip(True)
''')

assert_fail('''
"abc".ltrip(True)
''')

assert_fail('''
"abc".strip(cutset = '')
''')

assert_fail('''
"abc".rstrip(cutset = '')
''')

assert_fail('''
"abc".lstrip(cutset = '')
''')

