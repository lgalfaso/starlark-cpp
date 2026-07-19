words = ["able", "baker", "charlie"]
assert_eq({x: len(x) for x in words}, {"charlie": 7, "baker": 5, "able": 4})

