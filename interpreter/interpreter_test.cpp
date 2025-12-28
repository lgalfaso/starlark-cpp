// Copyright 2025 Lucas Mirelmann

#include <gmock/gmock.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <string>

#include "bigint/number.hpp"
#include "interpreter/interpreter.hpp"
#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_tuple.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::bigint::parse_number;
using ::starlark::interpreter::frame;
using ::starlark::interpreter::interpreter;
using ::starlark::logging::logger;
using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_bytes;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_float;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;
using ::starlark::testing::error_handler;
using ::testing::Contains;
using ::testing::IsEmpty;
using ::testing::Not;
using ::testing::SizeIs;

namespace {

std::string print_logs(logger& logging) {
  std::string result;
  for (const auto& entry : logging) {
    result += entry.ShortDebugString();
    result += "\n";
  }
  return result;
}

TEST(Interpreter, InvalidProgram) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a = [
)starlark", arena, logging);
  ASSERT_EQ(nullptr, result);
}

TEST(Interpreter, ExpressionStatements) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
[0]
1
0x1234567890abcdefabcdef
None
{'a': 2}
1.25
"abc"
b"def"
(1, 2, 3, 4)
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, IsEmpty());
  ASSERT_THAT(logging, IsEmpty());
}

TEST(Interpreter, Primitives) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a = [0]
b = 1
c = 0x1234567890abcdefabcdef
d = None
e = {'a': 2}
f = 1.25
g = "abc"
h = b"def"
i = (1, 2, 3, 4)
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(9));

  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_integer four(4);
  starlark_string string_a("a");
  error_handler error_callback;

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  starlark_list element_a;
  element_a.add(&zero, error_callback);
  EXPECT_TRUE(element_a.equals(*result->elements[0]));
  EXPECT_TRUE(starlark_integer(1).equals(*result->elements[1]));
  number n = parse_number("0x1234567890abcdefabcdef", nullptr);
  EXPECT_TRUE(starlark_bigint(n).equals(*result->elements[2]));
  EXPECT_TRUE(starlark_none().equals(*result->elements[3]));
  starlark_dictionary element_e;
  element_e.insert(&string_a, &two, error_callback);
  EXPECT_TRUE(element_e.equals(*result->elements[4]));
  EXPECT_TRUE(starlark_float(1.25).equals(*result->elements[5]));
  EXPECT_TRUE(starlark_string("abc").equals(*result->elements[6]));
  EXPECT_TRUE(starlark_bytes("def").equals(*result->elements[7]));
  starlark_tuple element_i;
  element_i.add(&one);
  element_i.add(&two);
  element_i.add(&three);
  element_i.add(&four);
  EXPECT_TRUE(element_i.equals(*result->elements[8]));
}

TEST(Interpreter, UseBeforeAssignment) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a = b
b = []
)starlark", arena, logging);
  ASSERT_EQ(nullptr, result);
  ASSERT_THAT(logging, SizeIs(1));
  EXPECT_EQ("UnboundLocalError: cannot access local variable 'b' where it is not associated with a value", logging.begin()->message());
}

TEST(Interpreter, NoDuplicateKeysInDictionaryLiterals) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a = {'a': 1, 'a': 2}
)starlark", arena, logging);
  ASSERT_EQ(nullptr, result);
  ASSERT_THAT(logging, SizeIs(1));
  EXPECT_EQ("Error: dictionary expression has duplicate key: \"a\"", logging.begin()->message());
}

TEST(Interpreter, ShortCircuit) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a = 0 and None
b = 0 and 1
c = 1 and None
d = 1 and 2
e = 0 or None
f = 0 or 1
g = 1 or None
h = 1 or 2
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(8));

  starlark_none none;
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  ASSERT_THAT(result->elements, Not(Contains(nullptr)));

  EXPECT_TRUE(zero.equals(*result->elements[0]));
  EXPECT_TRUE(zero.equals(*result->elements[1]));
  EXPECT_TRUE(none.equals(*result->elements[2]));
  EXPECT_TRUE(two.equals(*result->elements[3]));
  EXPECT_TRUE(none.equals(*result->elements[4]));
  EXPECT_TRUE(one.equals(*result->elements[5]));
  EXPECT_TRUE(one.equals(*result->elements[6]));
  EXPECT_TRUE(one.equals(*result->elements[7]));
}

TEST(Interpreter, SimpleCompoundAssignment) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
() = []
(a, b) = [0, 1]
[c, d] = (2, 3)
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(4));

  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_integer four(4);
  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_TRUE(zero.equals(*result->elements[0]));
  EXPECT_TRUE(one.equals(*result->elements[1]));
  EXPECT_TRUE(two.equals(*result->elements[2]));
  EXPECT_TRUE(three.equals(*result->elements[3]));
}

TEST(Interpreter, UnaryOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a = not True
b = not False
c = -1
d = -1.25
e = +1
f = +1.125
g = -123456789012345678901234567890
h = +123456789012345678901234567890
i = ~-1
j = ~123456789012345678901234567890
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(10));

  starlark_bool bool_true(true);
  starlark_bool bool_false(false);
  starlark_integer minus_one(-1);
  starlark_float minus_one_25(-1.25);
  starlark_integer one(1);
  starlark_float one_125(1.125);
  number n = parse_number("123456789012345678901234567890", nullptr);
  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_TRUE(bool_false.equals(*result->elements[0]));
  EXPECT_TRUE(bool_true.equals(*result->elements[1]));
  EXPECT_TRUE(minus_one.equals(*result->elements[2]));
  EXPECT_TRUE(minus_one_25.equals(*result->elements[3]));
  EXPECT_TRUE(one.equals(*result->elements[4]));
  EXPECT_TRUE(one_125.equals(*result->elements[5]));
  EXPECT_TRUE(starlark_bigint(-n).equals(*result->elements[6]));
  EXPECT_TRUE(starlark_bigint(n).equals(*result->elements[7]));
  EXPECT_TRUE(starlark_integer(0).equals(*result->elements[8]));
  EXPECT_TRUE(starlark_bigint(-(n + number::one)).equals(*result->elements[9]));
}

TEST(Interpreter, BinaryEqualsOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a = [] == []
b = [] == False
c = a != []
d = b != False
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(4));

  starlark_bool bool_true(true);
  starlark_bool bool_false(false);
  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_TRUE(bool_true.equals(*result->elements[0]));
  EXPECT_TRUE(bool_false.equals(*result->elements[1]));
  EXPECT_TRUE(bool_true.equals(*result->elements[2]));
  EXPECT_TRUE(bool_false.equals(*result->elements[3]));
}

TEST(Interpreter, BinaryLessThanOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a1 = False < True
a2 = True < False
b1 = 0 < 1
b2 = 1 < 0
c1 = 0.1 < 0.2
c2 = 0.2 < 0.1
c3 = 1e50 < 100000000000000007629769841091887003294964970946561
d1 = "" < "a"
d2 = "a" < ""
e1 = b"" < b"a"
e2 = b"a" < b""
f1 = () < (1,)
f2 = (1,) < ()
g1 = [] < [1]
g2 = [1] < []
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(15));

  starlark_bool bool_true(true);
  starlark_bool bool_false(false);
  ASSERT_THAT(result->elements, Not(Contains(nullptr)));

  EXPECT_TRUE(bool_true.equals(*result->elements[0]));
  EXPECT_TRUE(bool_false.equals(*result->elements[1]));
  EXPECT_TRUE(bool_true.equals(*result->elements[2]));
  EXPECT_TRUE(bool_false.equals(*result->elements[3]));
  EXPECT_TRUE(bool_true.equals(*result->elements[4]));
  EXPECT_TRUE(bool_false.equals(*result->elements[5]));
  EXPECT_TRUE(bool_true.equals(*result->elements[6]));
  EXPECT_TRUE(bool_true.equals(*result->elements[7]));
  EXPECT_TRUE(bool_false.equals(*result->elements[8]));
  EXPECT_TRUE(bool_true.equals(*result->elements[9]));
  EXPECT_TRUE(bool_false.equals(*result->elements[10]));
  EXPECT_TRUE(bool_true.equals(*result->elements[11]));
  EXPECT_TRUE(bool_false.equals(*result->elements[12]));
  EXPECT_TRUE(bool_true.equals(*result->elements[13]));
  EXPECT_TRUE(bool_false.equals(*result->elements[14]));
}

TEST(Interpreter, BinaryLessThanOperatorUncomparable) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a1 = False < 1
)starlark", arena, logging);
  ASSERT_EQ(nullptr, result);
}

TEST(Interpreter, BinaryLessThanOrEqualsOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a = 0 <= 0
b = 0 <= 1
c = 1 <= 0
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(3));

  starlark_bool bool_true(true);
  starlark_bool bool_false(false);
  ASSERT_THAT(result->elements, Not(Contains(nullptr)));

  EXPECT_TRUE(bool_true.equals(*result->elements[0]));
  EXPECT_TRUE(bool_true.equals(*result->elements[1]));
  EXPECT_TRUE(bool_false.equals(*result->elements[2]));
}

TEST(Interpreter, BinaryGreaterThanOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a = 0 > 0
b = 0 > 1
c = 1 > 0
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(3));

  starlark_bool bool_true(true);
  starlark_bool bool_false(false);
  ASSERT_THAT(result->elements, Not(Contains(nullptr)));

  EXPECT_TRUE(bool_false.equals(*result->elements[0]));
  EXPECT_TRUE(bool_false.equals(*result->elements[1]));
  EXPECT_TRUE(bool_true.equals(*result->elements[2]));
}

TEST(Interpreter, BinaryGreaterThanOrEqualsOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a = 0 >= 0
b = 0 >= 1
c = 1 >= 0
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(3));

  starlark_bool bool_true(true);
  starlark_bool bool_false(false);
  ASSERT_THAT(result->elements, Not(Contains(nullptr)));

  EXPECT_TRUE(bool_true.equals(*result->elements[0]));
  EXPECT_TRUE(bool_false.equals(*result->elements[1]));
  EXPECT_TRUE(bool_true.equals(*result->elements[2]));
}

TEST(Interpreter, BinaryMembershipOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = 1 in [1, 2, 3]
a02 = 4 not in (1, 2, 3)

d = {"one": 1, "two": 2}
a03 = "one" in d
a04 = "three" in d
a05 = 1 in d

a06 = "nasty" in "dynasty"
a07 = "a" in "banana"
a08 = "f" not in "way"

a09 = b"nasty" in b"dynasty"
a10 = 97 in b"abc"
a11 = 100 in b"abc"
# a12 = 1 in set([1, 2, 3])
# a13 = 1 in range(10)
)starlark", arena, logging);
  // TODO(lmirelmann): Add the test for `range` and `set`.
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(12));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_TRUE(result->elements[0]->truthy());
  EXPECT_TRUE(result->elements[1]->truthy());
  EXPECT_TRUE(result->elements[2]->truthy());
  EXPECT_FALSE(result->elements[3]->truthy());
  EXPECT_FALSE(result->elements[4]->truthy());
  EXPECT_TRUE(result->elements[5]->truthy());
  EXPECT_TRUE(result->elements[6]->truthy());
  EXPECT_TRUE(result->elements[7]->truthy());
  EXPECT_TRUE(result->elements[8]->truthy());
  EXPECT_TRUE(result->elements[9]->truthy());
  EXPECT_FALSE(result->elements[10]->truthy());
}

TEST(Interpreter, BinaryShiftOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = 1 << 10
a02 = 0xff0000 >> 4
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(2));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_TRUE(starlark_integer(1 << 10).equals(*result->elements[0]));
  EXPECT_TRUE(starlark_integer(0xff0000 >> 4).equals(*result->elements[1]));
}

TEST(Interpreter, BinaryPipeOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = 10 | 423
a02 = {1: 'one'} | {2: 'two'}
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(2));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_TRUE(starlark_integer(10 | 423).equals(*result->elements[0]));
  EXPECT_EQ(result->elements[1]->str(), "{1: \"one\", 2: \"two\"}");
}

TEST(Interpreter, BinaryAndOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = 10 & 423
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(1));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_TRUE(starlark_integer(10 & 423).equals(*result->elements[0]));
}

TEST(Interpreter, BinaryHatOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = 10 ^ 423
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(1));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_TRUE(starlark_integer(10 ^ 423).equals(*result->elements[0]));
}

TEST(Interpreter, BinaryPlusOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = (1,2) + (3,4)
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(1));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_EQ(result->elements[0]->str(), "(1, 2, 3, 4)");
}

TEST(Interpreter, BinaryMinusOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = 3 - 5
a02 = 4 - 5.0
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(2));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_EQ(result->elements[0]->str(), "-2");
  EXPECT_EQ(result->elements[1]->str(), "-1.0");
}

TEST(Interpreter, BinaryStarOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = (1,2) * 2
)starlark", arena, logging);
  ASSERT_NE(nullptr, result);
  ASSERT_THAT(result->elements, SizeIs(1));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_EQ(result->elements[0]->str(), "(1, 2, 1, 2)");
}

TEST(Interpreter, BinarySlashOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = 1 / 2
a02 = 3.0 / 2
)starlark", arena, logging);
  ASSERT_NE(nullptr, result) << print_logs(logging);
  ASSERT_THAT(result->elements, SizeIs(2));

  ASSERT_THAT(result->elements, Not(Contains(nullptr))) << print_logs(logging);
  EXPECT_EQ(result->elements[0]->str(), "0.5");
  EXPECT_EQ(result->elements[1]->str(), "1.5");
}

TEST(Interpreter, BinarySlashSlashOperator) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = 5 // 2
a02 = 3.0 // 2
)starlark", arena, logging);
  ASSERT_NE(nullptr, result) << print_logs(logging);
  ASSERT_THAT(result->elements, SizeIs(2));

  ASSERT_THAT(result->elements, Not(Contains(nullptr))) << print_logs(logging);
  EXPECT_EQ(result->elements[0]->str(), "2");
  EXPECT_EQ(result->elements[1]->str(), "1.0");
}

TEST(Interpreter, NotPossibleToHaveUnhashableKeys) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a = {[]: 1}
)starlark", arena, logging);
  ASSERT_EQ(nullptr, result);
  ASSERT_THAT(logging, SizeIs(1));
  EXPECT_EQ("TypeError: cannot use 'list' as a dict key (unhashable type: 'list')", logging.begin()->message());
}

TEST(Interpreter, Call) {
  interpreter runner;
  Arena arena;
  logger logging;

  // TODO(lmirelmann): Add tests for named arguments.
  // TODO(lmirelmann): Add tests for variadic positional arguments.
  // TODO(Lmirelmann): Add tests for variadic keyword arguments.
  frame* result = runner.run(R"starlark(
a01 = len([])
)starlark", arena, logging);
  ASSERT_NE(nullptr, result) << print_logs(logging);
  ASSERT_THAT(result->elements, SizeIs(1));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_EQ(result->elements[0]->str(), "0");
}

TEST(Interpreter, BuiltInFunctions) {
  interpreter runner;
  Arena arena;
  logger logging;

  // TODO(lmirelmann): Test all the built-in functions.
  frame* result = runner.run(R"starlark(
a00 = abs(-1)
a01 = any([True, False])
a02 = all([True, False])
a03 = bool(1)
a04 = bytes("abc")
# chr
# dict
# dir
# enumerate
# fail
# float
# getattr
# hasattr
# hash
# int
a15 = len([])
a16 = list((1, 2))
# max
# min
# ord
# print
# range
# repr
# reversed
# set
# sorted
# str
# tuple
# type
# zip
)starlark", arena, logging);
  ASSERT_NE(nullptr, result) << print_logs(logging);
  ASSERT_THAT(result->elements, SizeIs(7));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_EQ(result->elements[0]->str(), "1");
  EXPECT_EQ(result->elements[1]->str(), "True");
  EXPECT_EQ(result->elements[2]->str(), "False");
  EXPECT_EQ(result->elements[3]->str(), "True");
  EXPECT_EQ(result->elements[4]->str(), "b\"abc\"");
  EXPECT_EQ(result->elements[5]->str(), "0");
  EXPECT_EQ(result->elements[6]->str(), "[1, 2]");
}

TEST(Interpreter, ListComprehension) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = [x*x for x in [1,2,3,4]]
a02 = [x*y for x in [1,2,3,4] for y in [5, 6]]
a03 = [x*y for x in [1,2,3,4] if x % 2 == 1 for y in [5, 6]]
)starlark", arena, logging);
  ASSERT_NE(nullptr, result) << print_logs(logging);
  ASSERT_THAT(result->elements, SizeIs(3));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_EQ(result->elements[0]->str(), "[1, 4, 9, 16]");
  EXPECT_EQ(result->elements[1]->str(), "[5, 6, 10, 12, 15, 18, 20, 24]");
  EXPECT_EQ(result->elements[2]->str(), "[5, 6, 15, 18]");
}

TEST(Interpreter, DictionaryComprehension) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = {x: x*x for x in [1,2,3,4]}
a02 = {x: y for x in [1,2,3,4] for y in [5, 6]}
a03 = {x: y for x in [1,2,3,4] if x % 2 == 1 for y in [5, 6]}
)starlark", arena, logging);
  ASSERT_NE(nullptr, result) << print_logs(logging);
  ASSERT_THAT(result->elements, SizeIs(3));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_EQ(result->elements[0]->str(), "{1: 1, 2: 4, 3: 9, 4: 16}");
  EXPECT_EQ(result->elements[1]->str(), "{1: 6, 2: 6, 3: 6, 4: 6}");
  EXPECT_EQ(result->elements[2]->str(), "{1: 6, 3: 6}");
}

TEST(Interpreter, ExampleFloatingPointFromSpec) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
big = (1<<53)+1
a01 = (big + 0.0) == big
a02 = (big + 0.0) - big
)starlark", arena, logging);
  ASSERT_NE(nullptr, result) << print_logs(logging);
  ASSERT_THAT(result->elements, SizeIs(3));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_EQ(result->elements[0]->str(), "False");
  EXPECT_EQ(result->elements[1]->str(), "0.0");
  EXPECT_EQ(result->elements[2]->str(), "9007199254740993");
}

TEST(Interpreter, IndexMember) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = [1, 2, 3, 4]
a02 = a01[2]
)starlark", arena, logging);
  ASSERT_NE(nullptr, result) << print_logs(logging);
  ASSERT_THAT(result->elements, SizeIs(2));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_EQ(result->elements[0]->str(), "[1, 2, 3, 4]");
  EXPECT_EQ(result->elements[1]->str(), "3");
}

TEST(Interpreter, IndexMemberAssignment) {
  interpreter runner;
  Arena arena;
  logger logging;

  frame* result = runner.run(R"starlark(
a01 = [1, 2, 3, 4]
a01[2] = 100
a02 = {'a': 1, 'b': 2}
a02['b'] = 100
a02['c'] = 101
)starlark", arena, logging);
  ASSERT_NE(nullptr, result) << print_logs(logging);
  ASSERT_THAT(result->elements, SizeIs(2));

  ASSERT_THAT(result->elements, Not(Contains(nullptr)));
  EXPECT_EQ(result->elements[0]->str(), "[1, 2, 100, 4]");
  EXPECT_EQ(result->elements[1]->str(), "{\"a\": 1, \"b\": 100, \"c\": 101}");
}

}  // namespace

