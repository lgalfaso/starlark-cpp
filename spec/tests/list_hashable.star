assert_fail('''{[]: 1}''')

# List is not hashable even if freezed.
assert_fail('''
## Begin module: "//:test1.bzl"
a = []
## End module
## Main
load("//:test1.bzl", "a")
{a: 1}
''')

assert_fail('''
r1 = []
r2 = [r1]
r1.append(r2)
{r1: None, r2: None}
''')

