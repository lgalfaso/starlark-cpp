reserved_words = [
    "as",       "assert",   "async",    "await",  "class",    "del",      "except",   "finally",
    "from",     "global",   "import",   "is",     "nonlocal", "raise",    "try",      "while",
    "with",     "yield",
]

template = """
%s = 1
"""

assert_succeed(template % "x")
[
  assert_fail(template % rw)
  for rw in reserved_words
]
