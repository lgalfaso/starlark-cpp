// Copyright 2025 Lucas Mirelmann

#include "compiler/siphash.hpp"

#include <bit>

namespace starlark {
namespace compiler {

/* default: SipHash-1-3 */
#ifndef cROUNDS
#define cROUNDS 1
#endif
#ifndef dROUNDS
#define dROUNDS 3
#endif

#define ROTL(x, b) std::rotl(x, b)

#define U8TO64_LE(p)                                                                    \
    ((static_cast<uint64_t>((p)[0])) | (static_cast<uint64_t>((p)[1]) << 8) |           \
     (static_cast<uint64_t>((p)[2]) << 16) | (static_cast<uint64_t>((p)[3]) << 24) |    \
     (static_cast<uint64_t>((p)[4]) << 32) | (static_cast<uint64_t>((p)[5]) << 40) |    \
     (static_cast<uint64_t>((p)[6]) << 48) | (static_cast<uint64_t>((p)[7]) << 56))

#define SIPROUND                                                               \
    {                                                                          \
        v0 += v1;                                                              \
        v1 = ROTL(v1, 13);                                                     \
        v1 ^= v0;                                                              \
        v0 = ROTL(v0, 32);                                                     \
        v2 += v3;                                                              \
        v3 = ROTL(v3, 16);                                                     \
        v3 ^= v2;                                                              \
        v0 += v3;                                                              \
        v3 = ROTL(v3, 21);                                                     \
        v3 ^= v0;                                                              \
        v2 += v1;                                                              \
        v1 = ROTL(v1, 17);                                                     \
        v1 ^= v2;                                                              \
        v2 = ROTL(v2, 32);                                                     \
    }

uint64_t siphash(const char *in, const size_t inlen, uint64_t k0, uint64_t k1) {
  const unsigned char* input = reinterpret_cast<const unsigned char*>(in);
  uint64_t v0 = 0x736f6d6570736575ul;
  uint64_t v1 = 0x646f72616e646f6dul;
  uint64_t v2 = 0x6c7967656e657261ul;
  uint64_t v3 = 0x7465646279746573ul;
  uint64_t m;
  int i;
  const unsigned char *end = input + inlen - (inlen % sizeof(uint64_t));
  const int left = inlen & 7;
  uint64_t b = static_cast<uint64_t>(inlen) << 56;

  v3 ^= k1;
  v2 ^= k0;
  v1 ^= k1;
  v0 ^= k0;

  for (; input != end; input += 8) {
    m = U8TO64_LE(input);
    v3 ^= m;

    for (i = 0; i < cROUNDS; ++i) {
      SIPROUND;
    }

    v0 ^= m;
  }

  switch (left) {
    case 7:
      b |= static_cast<uint64_t>(input[6]) << 48;
      /* FALLTHRU */
    case 6:
      b |= static_cast<uint64_t>(input[5]) << 40;
      /* FALLTHRU */
    case 5:
      b |= static_cast<uint64_t>(input[4]) << 32;
      /* FALLTHRU */
    case 4:
      b |= static_cast<uint64_t>(input[3]) << 24;
      /* FALLTHRU */
    case 3:
      b |= static_cast<uint64_t>(input[2]) << 16;
      /* FALLTHRU */
    case 2:
      b |= static_cast<uint64_t>(input[1]) << 8;
      /* FALLTHRU */
    case 1:
      b |= static_cast<uint64_t>(input[0]);
      break;
    case 0:
      break;
  }

  v3 ^= b;

  for (i = 0; i < cROUNDS; ++i) {
    SIPROUND;
  }

  v0 ^= b;
  v2 ^= 0xff;
  for (i = 0; i < dROUNDS; ++i) {
    SIPROUND;
  }
  return v0 ^ v1 ^ v2 ^ v3;
}

}  // namespace compiler
}  // namespace starlark


