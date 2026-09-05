assert_eq(1 in [1, 2, 3], True)
assert_eq(4 not in (1, 2, 3), True)

d = {"one": 1, "two": 2}
assert_eq("one" in d, True)
assert_eq("three" in d, False)
assert_eq(1 in d, False)

assert_eq("nasty" in "dynasty", True)
assert_eq("a" in "banana", True)
assert_eq("f" not in "way", True)

assert_eq(b"nasty" in b"dynasty", True)
assert_eq(97 in b"abc", True)
assert_eq(100 in b"abc", False)

assert_eq(1 in set([1, 2, 3]), True)
assert_eq(1 in range(10), True)

