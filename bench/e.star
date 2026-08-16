"""Pure-Starlark computation of decimal digits of e (Taylor series, integer-only)."""

# Extra fixed-point digits for rounding before truncation.
_ROUND_GUARD = 2

# First 200 decimal digits of e (including leading 2), for correctness checks.
_E_KNOWN_PREFIX = (
    "27182818284590452353602874713526624977572470936999595749669676277240766303535475" +
    "94571382178525166427427466391932003059921817413596629043572900334295260595630738" +
    "1323286279434907632338298807531952510190"
)

def _pow10(n):
    if n < 0:
        fail("_pow10: n must be >= 0, got %d" % n)
    p = 1
    for _ in range(n):
        p *= 10
    return p

def e_terms_cap(n_digits):
    """Safe upper bound on Taylor terms (1/k!) for n_digits decimal output."""
    if n_digits < 0:
        fail("e_terms_cap: n_digits must be >= 0, got %d" % n_digits)
    if n_digits == 0:
        return 0
    return n_digits + _ROUND_GUARD + 8

def e_known_prefix(n_digits):
    """Reference prefix of e (empty when n_digits == 0)."""
    if n_digits <= 0:
        return ""
    if n_digits > len(_E_KNOWN_PREFIX):
        fail("e_known_prefix: only %d reference digits available, got %d" % (
            len(_E_KNOWN_PREFIX),
            n_digits,
        ))
    return _E_KNOWN_PREFIX[:n_digits]

def e_digits(n_digits):
    """Return the first n_digits decimal digits of e.

    Digits are returned without a decimal point, starting with 2
    (e = 2.71828... -> e_digits(5) == "27182").
    """

    if n_digits <= 0:
        return ""

    scale = _pow10(n_digits + _ROUND_GUARD)
    term = scale
    total = term

    for k in range(1, e_terms_cap(n_digits)):
        term = term // k
        if term == 0:
            break
        total += term

    half = 5 * _pow10(_ROUND_GUARD - 1)
    rounded = (total + half) // _pow10(_ROUND_GUARD)
    s = str(rounded)
    if len(s) > n_digits:
        s = s[:n_digits]
    elif len(s) < n_digits:
        s = ("0" * (n_digits - len(s))) + s
    return s


_BENCH_SIZES = [1, 5, 10, 20, 50, 80, 100, 200]

def run():
    for n in _BENCH_SIZES:
        e = e_digits(n)
        print(e)

run()
