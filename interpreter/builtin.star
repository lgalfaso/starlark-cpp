def max_impl(*args, key=None):
  if len(args) == 0:
    fail("TypeError: max expected at least 1 argument, got 0")
  if len(args) == 1:
    base = list(args[0])
  else:
    base = args
  keys = [(key(x) if key != None else x) for x in base]
  return inner_max(base, keys)

