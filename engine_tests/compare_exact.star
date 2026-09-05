big = (1<<53)+1
assert_eq((big + 0.0) == big, False)
assert_eq((big + 0.0) - big, 0.0)

