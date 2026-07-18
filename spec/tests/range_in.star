assert_true(1 in range(100))
assert_false(100 in range(100))
assert_false(False in range(100))
assert_true(1.0 in range(100))

imin = -1 << 63
imax = (1 << 63) - 1
inf = 1e308 * 10
nan = inf / inf

assert_false(-1 in range(0, 100, 1))
assert_true(0 in range(0, 100, 1))
assert_true(1 in range(0, 100, 1))
assert_true(99 in range(0, 100, 1))
assert_false(100 in range(0, 100, 1))
assert_false(0 in range(100, 0, -1))
assert_true(1 in range(100, 0, -1))
assert_true(99 in range(100, 0, -1))
assert_true(100 in range(100, 0, -1))
assert_false(101 in range(100, 0, -1))

assert_false(imin in range(-1, 100, 1))
assert_false(imax in range(-1, 100, 1))
assert_false(imin in range(0, 100, 1))
assert_false(imax in range(0, 100, 1))
assert_false(imin in range(1, 100, 1))
assert_false(imax in range(1, 100, 1))

assert_false(imin in range(100, -1, -1))
assert_false(imax in range(100, -1, -1))
assert_false(imin in range(100, 0, -1))
assert_false(imax in range(100, 0, -1))
assert_false(imin in range(100, 1, -1))
assert_false(imax in range(100, 1, -1))

assert_false(imin in range(imin + 1, 0, 1))
assert_false(imax in range(0, imax - 1, 1))
assert_false(imax in range(imax - 1, 0, -1))
assert_false(imin in range(0, imin + 1, -1))

assert_false(imin + 1 in range(imin, imax, 3))
assert_false(imin + 2 in range(imin, imax, 3))
assert_true(imin + 3 in range(imin, imax, 3))
assert_false(imax - 1 in range(imin, imax, 3))
assert_true(imax - 2 in range(imin, imax, 3))
assert_false(imax - 3 in range(imin, imax, 3))

assert_false((1 << 63) - 1 in range(0, imax, 1))
assert_false((1 << 63) in range(0, imax, 1))
assert_false((1 << 64) in range(0, imax, 1))


assert_false(-1.0 in range(0, 100, 1))
assert_false(-0.5 in range(0, 100, 1))
assert_true(-0.0 in range(0, 100, 1))
assert_true(0.0 in range(0, 100, 1))
assert_false(0.5 in range(0, 100, 1))
assert_true(1.0 in range(0, 100, 1))
assert_false(1.5 in range(0, 100, 1))
assert_false(98.5 in range(0, 100, 1))
assert_true(99.0 in range(0, 100, 1))
assert_false(99.5 in range(0, 100, 1))
assert_false(100.0 in range(0, 100, 1))
assert_false(100.5 in range(0, 100, 1))
assert_false(-1e308 in range(0, 100, 1))
assert_false(1e308 in range(0, 100, 1))
assert_false(-inf in range(0, 100, 1))
assert_false(inf in range(0, 100, 1))
assert_false(nan in range(0, 100, 1))


assert_false(-0.5 in range(100, 0, -1))
assert_false(-0.0 in range(100, 0, -1))
assert_false(0.0 in range(100, 0, -1))
assert_false(0.5 in range(100, 0, -1))
assert_true(1.0 in range(100, 0, -1))
assert_false(1.5 in range(100, 0, -1))
assert_false(98.5 in range(100, 0, -1))
assert_true(99.0 in range(100, 0, -1))
assert_false(99.5 in range(100, 0, -1))
assert_true(100.0 in range(100, 0, -1))
assert_false(100.5 in range(100, 0, -1))
assert_false(101.0 in range(100, 0, -1))
assert_false(101.5 in range(100, 0, -1))

assert_false(None in range(0, 100, 1))
assert_false(True in range(0, 100, 1))
assert_false([] in range(0, 100, 1))

