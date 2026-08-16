"""Pure-Starlark base64 encode/decode (RFC 4648, standard alphabet)."""

_BASE64_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

_REVERSE_ALPHABET = {
  ch: pos for pos, ch in enumerate(_BASE64_ALPHABET.elems())
}

def _alphabet_index(ch):
    return _REVERSE_ALPHABET.get(ch, -1)

def _encode_quad(b0, b1, b2, rem):
    """Encode up to three bytes; rem is how many input bytes are valid (1..3)."""
    triple = (b0 << 16) | (b1 << 8) | b2
    c0 = _BASE64_ALPHABET[(triple >> 18) & 63]
    c1 = _BASE64_ALPHABET[(triple >> 12) & 63]
    if rem == 1:
        return [c0, c1, "=", "="]
    c2 = _BASE64_ALPHABET[(triple >> 6) & 63]
    if rem == 2:
        return [c0, c1, c2, "="]
    c3 = _BASE64_ALPHABET[triple & 63]
    return [c0, c1, c2, c3]

def base64_encode(data):
    """Encode a bytes to a base64 string.

    Returns (encoded_string).
    """
    n = len(data)
    if n == 0:
        return ""

    out = []
    for i in range(0, n, 3):
        b0 = ord(data[i])
        b1 = 0
        b2 = 0
        rem = 1
        if i + 1 < n:
            b1 = ord(data[i + 1])
            rem = 2
        if i + 2 < n:
            b2 = ord(data[i + 2])
            rem = 3
        quad = _encode_quad(b0, b1, b2, rem)
        for ch in quad:
            out.append(ch)
    return "".join(out)

def base64_decode(encoded):
    """Decode a base64 string to a list of byte values (0..255).

    Returns (data). Fails on invalid padding or characters.
    """
    n = len(encoded)
    if n == 0:
        return b''
    if n % 4 != 0:
        fail("base64_decode: length %d is not a multiple of 4" % n)

    out = []
    for i in range(0, n, 4):
        vals = []
        pad = 0
        for j in range(4):
            ch = encoded[i + j]
            if ch == "=":
                pad += 1
                if pad > 2 or j < 2:
                    fail("base64_decode: invalid padding at index %d" % (i + j))
                vals.append(0)
                continue
            if pad > 0:
                fail("base64_decode: data after padding at index %d" % (i + j))
            val = _alphabet_index(ch)
            if val < 0:
                fail("base64_decode: invalid character %r" % ch)
            vals.append(val)
        if len(vals) != 4:
            fail("base64_decode: incomplete quad at index %d" % i)
        triple = (vals[0] << 18) | (vals[1] << 12) | (vals[2] << 6) | vals[3]
        out.append((triple >> 16) & 255)
        if pad < 2:
            out.append((triple >> 8) & 255)
        if pad == 0:
            out.append(triple & 255)
    return bytes(out)


_BENCH_SIZES = [0, 10, 30, 100, 1000, 5000]

# Knuth multiplicative hash for deterministic pseudo-random byte payloads.
_HASH_MIX = 2654435761

def deterministic_payload(n):
    """Build n pseudo-random bytes (deterministic)."""
    data = []
    for i in range(n):
        data.append((i * _HASH_MIX) % 256)
    return bytes(data)

def run():
    for n in _BENCH_SIZES:
        data = deterministic_payload(n)
        encoded = base64_encode(data)
        decoded = base64_decode(encoded)
        if decoded != data:
              fail("round trip fail")

run()
