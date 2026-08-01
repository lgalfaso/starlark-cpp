# The `load` statement can only show up at the top of the module. The only exception is that it can be after a string literal.

a = 1
# This will be a static error.
load("//:test.bzl", "foo")

