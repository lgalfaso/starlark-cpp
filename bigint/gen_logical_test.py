import argparse

def numbers(n, d):
  base = [0]
  for a in range(1, d+ 1):
    base.append(a)
    base.append((1<<64) -a)
  nbase = [(f'{s:018x}')[2:] for s in base]
  nums = []
  for a in range(n):
    nums = [a + b for a in nums or [""] for b in nbase]
  return sorted([int(s + num, 16) for s in ["", "-"] for num in nums])

def makeBinaryTest(testName, operator, fn):
  result = []
  block = 0
  nums = numbers(3, 2)
  for a in nums:
    result.append("TEST(Number, %s%d) {" % (testName, block))
    for b in nums:
      result.append("  EXPECT_EQ(parse_number(\"%s\", nullptr), parse_number(\"%s\", nullptr).%s(parse_number(\"%s\", nullptr)));" % (hex(fn(a,b)), hex(a), operator, hex(b)))
    result.append("}")
    result.append("")
    block = block + 1
  return "\n".join(result)

def makeUnaryTest(testName, operator, fn):
  result = []
  result.append("TEST(Number, %s) {" % testName)
  nums = numbers(3, 2)
  for a in nums:
    result.append("  EXPECT_EQ(parse_number(\"%s\", nullptr), parse_number(\"%s\", nullptr).%s());" % (hex(fn(a)), hex(a), operator))
  result.append("}")
  result.append("")
  return "\n".join(result)

parser = argparse.ArgumentParser("gen_logical_test")
parser.add_argument('--operator', choices=["all", "or", "and", "xor", "not"], default="all", nargs='?')
args = parser.parse_args()

print("""// Copyright 2025 Lucas Mirelmann
// Do not edit -- generated file.

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "bigint/number.hpp"
#include "bigint/number_stream_for_test.hpp"

using ::starlark::bigint::parse_number;

namespace {
""")

if args.operator == "all" or args.operator == "or":
  print(makeBinaryTest("LogicalOr", "logical_or", lambda a, b: a | b))

if args.operator == "all" or args.operator == "and":
  print(makeBinaryTest("LogicalAnd", "logical_and", lambda a, b: a & b))

if args.operator == "all" or args.operator == "xor":
  print(makeBinaryTest("LogicalXor", "logical_xor", lambda a, b: a ^ b))

if args.operator == "all" or args.operator == "not":
  print(makeUnaryTest("LogicalNot", "logical_not", lambda a: ~a))

print("""
}  // namespace

""")
