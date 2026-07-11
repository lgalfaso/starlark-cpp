assert_eq(str([]), '[]')
assert_eq(str([1]), '[1]')
assert_eq(str([1, 2]), '[1, 2]')

r = []
r.append(r)
r1 = []
r2 = [r1]
r1.append(r2)
assert_eq(str(r), '[[...]]')
assert_eq(str(r1), '[[[...]]]')

