calls = []
d = []
dd = {}
def f(name, result):
    calls.append(name)
    d.append(str(dd))
    return result

f("lhs1", dd)[0], f("lhs2", dd)[0] = f("rhs1", 0), f("rhs2", 0)
assert_eq(calls, ['rhs1', 'rhs2', 'lhs1', 'lhs2'])
assert_eq(dd, {0: 0})
assert_eq(d, ['{}', '{}', '{}', '{0: 0}'])

calls.clear()
d.clear()
dd.clear()

f("array", [0])[f("index", 0)] += f("addend", 1)
assert_eq(calls, ['array', 'index', 'addend'])

