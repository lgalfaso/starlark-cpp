a = 1
b = 1
c = 1
d = 1
e = 1
a if b else c if d else e
a if b else (c if d else e)          # parens are redundant
(a if b else c) if d else e          # parens are required

lambda: a if b else c
lambda: (a if b else c)              # parens are redunant
(lambda: a) if b else c              # parens are required

a if b else lambda: c if d else e
a if b else lambda: (c if d else e)  # parens are redundant
a if b else (lambda: c if d else e)  # parens are required
(a if b else lambda: c) if d else e  # parens are required
