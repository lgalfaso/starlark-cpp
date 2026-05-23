// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string_view>

#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_testing.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::parse_number;
using ::starlark::runtime::context;
using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_string;
using ::starlark::testing::error_handler;
using ::std::literals::string_view_literals::operator""sv;
using ::testing::IsEmpty;
using ::testing::SizeIs;

namespace {

TEST(StarlarkStringElems, Type) {
  Arena arena;
  context ctx(arena);

  EXPECT_EQ("string.elems", starlark_string(""sv).elems(ctx)->type());
  EXPECT_EQ("string.elem_ords", starlark_string(""sv).elem_ords(ctx)->type());
  EXPECT_EQ("string.codepoints", starlark_string(""sv).codepoints(ctx)->type());
  EXPECT_EQ("string.codepoint_ords", starlark_string(""sv).codepoint_ords(ctx)->type());
}

TEST(StarlarkStringElems, Truthy) {
  Arena arena;
  context ctx(arena);

  EXPECT_FALSE(starlark_string(""sv).elems(ctx)->truthy());
  EXPECT_TRUE(starlark_string("a"sv).elems(ctx)->truthy());
  EXPECT_FALSE(starlark_string(""sv).elem_ords(ctx)->truthy());
  EXPECT_TRUE(starlark_string("a"sv).elem_ords(ctx)->truthy());
  EXPECT_FALSE(starlark_string(""sv).codepoints(ctx)->truthy());
  EXPECT_TRUE(starlark_string("a"sv).codepoints(ctx)->truthy());
  EXPECT_FALSE(starlark_string(""sv).codepoint_ords(ctx)->truthy());
  EXPECT_TRUE(starlark_string("a"sv).codepoint_ords(ctx)->truthy());
}

TEST(StarlarkStringElems, Len) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(starlark_string(""sv).elems(ctx)->len(true, error_callback), 0);
  EXPECT_EQ(starlark_string("a"sv).elems(ctx)->len(true, error_callback), 1);
  EXPECT_EQ(starlark_string("ab"sv).elems(ctx)->len(true, error_callback), 2);
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).elems(ctx)->len(true, error_callback), 11);

  EXPECT_EQ(starlark_string(""sv).elem_ords(ctx)->len(true, error_callback), 0);
  EXPECT_EQ(starlark_string("a"sv).elem_ords(ctx)->len(true, error_callback), 1);
  EXPECT_EQ(starlark_string("ab"sv).elem_ords(ctx)->len(true, error_callback), 2);
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).elem_ords(ctx)->len(true, error_callback), 11);

  EXPECT_EQ(starlark_string(""sv).codepoints(ctx)->len(true, error_callback), 0);
  EXPECT_EQ(starlark_string("a"sv).codepoints(ctx)->len(true, error_callback), 1);
  EXPECT_EQ(starlark_string("ab"sv).codepoints(ctx)->len(true, error_callback), 2);
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).codepoints(ctx)->len(true, error_callback), 11);

  EXPECT_EQ(starlark_string(""sv).codepoint_ords(ctx)->len(true, error_callback), 0);
  EXPECT_EQ(starlark_string("a"sv).codepoint_ords(ctx)->len(true, error_callback), 1);
  EXPECT_EQ(starlark_string("ab"sv).codepoint_ords(ctx)->len(true, error_callback), 2);
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).codepoint_ords(ctx)->len(true, error_callback), 11);

  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkStringElems, Hash) {
  Arena arena;
  context ctx(arena);

  EXPECT_EQ(starlark_string(""sv).elems(ctx)->hash(), -1);
  EXPECT_EQ(starlark_string(""sv).elem_ords(ctx)->hash(), -1);
  EXPECT_EQ(starlark_string(""sv).codepoints(ctx)->hash(), -1);
  EXPECT_EQ(starlark_string(""sv).codepoint_ords(ctx)->hash(), -1);
}

TEST(StarlarkStringElems, Str) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(starlark_string("abc"sv).elems(ctx)->str(), "\"abc\".elems()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).elems(ctx)->str(), "\"Περιπτώσεις\".elems()");
  EXPECT_EQ(starlark_string("abc"sv).elem_ords(ctx)->str(), "\"abc\".elem_ords()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).elem_ords(ctx)->str(), "\"Περιπτώσεις\".elem_ords()");
  EXPECT_EQ(starlark_string("abc"sv).codepoints(ctx)->str(), "\"abc\".codepoints()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).codepoints(ctx)->str(), "\"Περιπτώσεις\".codepoints()");
  EXPECT_EQ(starlark_string("abc"sv).codepoint_ords(ctx)->str(), "\"abc\".codepoint_ords()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).codepoint_ords(ctx)->str(), "\"Περιπτώσεις\".codepoint_ords()");
}

TEST(StarlarkStringElems, SliceRange) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(starlark_string("abc"sv).elems(ctx)->slice_range(*ctx.one(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback)->str(), "\"bc\".elems()");
  EXPECT_EQ(starlark_string("abc"sv).elems(ctx)->slice_range(*ctx.none_value(), *ctx.one(), *ctx.none_value(), ctx, error_callback)->str(), "\"a\".elems()");
  EXPECT_EQ(starlark_string("abc"sv).elems(ctx)->slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.minus_one(), ctx, error_callback)->str(), "\"cba\".elems()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).elems(ctx)->slice_range(*ctx.one(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback)->str(), "\"εριπτώσεις\".elems()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).elems(ctx)->slice_range(*ctx.none_value(), *ctx.one(), *ctx.none_value(), ctx, error_callback)->str(), "\"Π\".elems()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).elems(ctx)->slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.minus_one(), ctx, error_callback)->str(), "\"ςιεσώτπιρεΠ\".elems()");

  EXPECT_EQ(starlark_string("abc"sv).elem_ords(ctx)->slice_range(*ctx.one(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback)->str(), "\"bc\".elem_ords()");
  EXPECT_EQ(starlark_string("abc"sv).elem_ords(ctx)->slice_range(*ctx.none_value(), *ctx.one(), *ctx.none_value(), ctx, error_callback)->str(), "\"a\".elem_ords()");
  EXPECT_EQ(starlark_string("abc"sv).elem_ords(ctx)->slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.minus_one(), ctx, error_callback)->str(), "\"cba\".elem_ords()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).elem_ords(ctx)->slice_range(*ctx.one(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback)->str(), "\"εριπτώσεις\".elem_ords()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).elem_ords(ctx)->slice_range(*ctx.none_value(), *ctx.one(), *ctx.none_value(), ctx, error_callback)->str(), "\"Π\".elem_ords()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).elem_ords(ctx)->slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.minus_one(), ctx, error_callback)->str(), "\"ςιεσώτπιρεΠ\".elem_ords()");

  EXPECT_EQ(starlark_string("abc"sv).codepoints(ctx)->slice_range(*ctx.one(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback)->str(), "\"bc\".codepoints()");
  EXPECT_EQ(starlark_string("abc"sv).codepoints(ctx)->slice_range(*ctx.none_value(), *ctx.one(), *ctx.none_value(), ctx, error_callback)->str(), "\"a\".codepoints()");
  EXPECT_EQ(starlark_string("abc"sv).codepoints(ctx)->slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.minus_one(), ctx, error_callback)->str(), "\"cba\".codepoints()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).codepoints(ctx)->slice_range(*ctx.one(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback)->str(), "\"εριπτώσεις\".codepoints()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).codepoints(ctx)->slice_range(*ctx.none_value(), *ctx.one(), *ctx.none_value(), ctx, error_callback)->str(), "\"Π\".codepoints()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).codepoints(ctx)->slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.minus_one(), ctx, error_callback)->str(), "\"ςιεσώτπιρεΠ\".codepoints()");

  EXPECT_EQ(starlark_string("abc"sv).codepoint_ords(ctx)->slice_range(*ctx.one(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback)->str(), "\"bc\".codepoint_ords()");
  EXPECT_EQ(starlark_string("abc"sv).codepoint_ords(ctx)->slice_range(*ctx.none_value(), *ctx.one(), *ctx.none_value(), ctx, error_callback)->str(), "\"a\".codepoint_ords()");
  EXPECT_EQ(starlark_string("abc"sv).codepoint_ords(ctx)->slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.minus_one(), ctx, error_callback)->str(), "\"cba\".codepoint_ords()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).codepoint_ords(ctx)->slice_range(*ctx.one(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback)->str(), "\"εριπτώσεις\".codepoint_ords()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).codepoint_ords(ctx)->slice_range(*ctx.none_value(), *ctx.one(), *ctx.none_value(), ctx, error_callback)->str(), "\"Π\".codepoint_ords()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).codepoint_ords(ctx)->slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.minus_one(), ctx, error_callback)->str(), "\"ςιεσώτπιρεΠ\".codepoint_ords()");

  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkStringElems, SliceRangeWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(nullptr, starlark_string("abc"sv).elems(ctx)->slice_range(*ctx.true_value(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
}

TEST(StarlarkStringElemOrds, SliceRangeWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(nullptr, starlark_string("abc"sv).elem_ords(ctx)->slice_range(*ctx.true_value(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
}

TEST(StarlarkStringCodepoints, SliceRangeWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(nullptr, starlark_string("abc"sv).codepoints(ctx)->slice_range(*ctx.true_value(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
}

TEST(StarlarkStringCodepointOrds, SliceRangeWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(nullptr, starlark_string("abc"sv).codepoint_ords(ctx)->slice_range(*ctx.true_value(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
}

TEST(StarlarkStringElems, Index) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  auto* elems = str.elems(ctx);
  auto* elem_ords = str.elem_ords(ctx);
  auto* codepoints = str.codepoints(ctx);
  auto* codepoint_ords = str.codepoint_ords(ctx);

  EXPECT_EQ(elems->index(starlark_integer(-3), ctx, error_callback)->repr(), "\"a\"");
  EXPECT_EQ(elems->index(starlark_integer(-2), ctx, error_callback)->repr(), "\"b\"");
  EXPECT_EQ(elems->index(starlark_integer(-1), ctx, error_callback)->repr(), "\"c\"");
  EXPECT_EQ(elems->index(starlark_integer(0), ctx, error_callback)->repr(), "\"a\"");
  EXPECT_EQ(elems->index(starlark_integer(1), ctx, error_callback)->repr(), "\"b\"");
  EXPECT_EQ(elems->index(starlark_integer(2), ctx, error_callback)->repr(), "\"c\"");

  EXPECT_EQ(elem_ords->index(starlark_integer(-3), ctx, error_callback)->repr(), "97");
  EXPECT_EQ(elem_ords->index(starlark_integer(-2), ctx, error_callback)->repr(), "98");
  EXPECT_EQ(elem_ords->index(starlark_integer(-1), ctx, error_callback)->repr(), "99");
  EXPECT_EQ(elem_ords->index(starlark_integer(0), ctx, error_callback)->repr(), "97");
  EXPECT_EQ(elem_ords->index(starlark_integer(1), ctx, error_callback)->repr(), "98");
  EXPECT_EQ(elem_ords->index(starlark_integer(2), ctx, error_callback)->repr(), "99");

  EXPECT_EQ(codepoints->index(starlark_integer(-3), ctx, error_callback)->repr(), "\"a\"");
  EXPECT_EQ(codepoints->index(starlark_integer(-2), ctx, error_callback)->repr(), "\"b\"");
  EXPECT_EQ(codepoints->index(starlark_integer(-1), ctx, error_callback)->repr(), "\"c\"");
  EXPECT_EQ(codepoints->index(starlark_integer(0), ctx, error_callback)->repr(), "\"a\"");
  EXPECT_EQ(codepoints->index(starlark_integer(1), ctx, error_callback)->repr(), "\"b\"");
  EXPECT_EQ(codepoints->index(starlark_integer(2), ctx, error_callback)->repr(), "\"c\"");

  EXPECT_EQ(codepoint_ords->index(starlark_integer(-3), ctx, error_callback)->repr(), "97");
  EXPECT_EQ(codepoint_ords->index(starlark_integer(-2), ctx, error_callback)->repr(), "98");
  EXPECT_EQ(codepoint_ords->index(starlark_integer(-1), ctx, error_callback)->repr(), "99");
  EXPECT_EQ(codepoint_ords->index(starlark_integer(0), ctx, error_callback)->repr(), "97");
  EXPECT_EQ(codepoint_ords->index(starlark_integer(1), ctx, error_callback)->repr(), "98");
  EXPECT_EQ(codepoint_ords->index(starlark_integer(2), ctx, error_callback)->repr(), "99");

  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkStringElems, IndexUnicode) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("Περιπτώσεις"sv);
  auto* elems = str.elems(ctx);
  auto* elem_ords = str.elem_ords(ctx);
  auto* codepoints = str.codepoints(ctx);
  auto* codepoint_ords = str.codepoint_ords(ctx);

  EXPECT_EQ(elems->index(starlark_integer(-3), ctx, error_callback)->repr(), "\"ε\"");
  EXPECT_EQ(elems->index(starlark_integer(-2), ctx, error_callback)->repr(), "\"ι\"");
  EXPECT_EQ(elems->index(starlark_integer(-1), ctx, error_callback)->repr(), "\"ς\"");
  EXPECT_EQ(elems->index(starlark_integer(0), ctx, error_callback)->repr(), "\"Π\"");
  EXPECT_EQ(elems->index(starlark_integer(1), ctx, error_callback)->repr(), "\"ε\"");
  EXPECT_EQ(elems->index(starlark_integer(2), ctx, error_callback)->repr(), "\"ρ\"");

  EXPECT_EQ(elem_ords->index(starlark_integer(-3), ctx, error_callback)->repr(), "949");
  EXPECT_EQ(elem_ords->index(starlark_integer(-2), ctx, error_callback)->repr(), "953");
  EXPECT_EQ(elem_ords->index(starlark_integer(-1), ctx, error_callback)->repr(), "962");
  EXPECT_EQ(elem_ords->index(starlark_integer(0), ctx, error_callback)->repr(), "928");
  EXPECT_EQ(elem_ords->index(starlark_integer(1), ctx, error_callback)->repr(), "949");
  EXPECT_EQ(elem_ords->index(starlark_integer(2), ctx, error_callback)->repr(), "961");

  EXPECT_EQ(codepoints->index(starlark_integer(-3), ctx, error_callback)->repr(), "\"ε\"");
  EXPECT_EQ(codepoints->index(starlark_integer(-2), ctx, error_callback)->repr(), "\"ι\"");
  EXPECT_EQ(codepoints->index(starlark_integer(-1), ctx, error_callback)->repr(), "\"ς\"");
  EXPECT_EQ(codepoints->index(starlark_integer(0), ctx, error_callback)->repr(), "\"Π\"");
  EXPECT_EQ(codepoints->index(starlark_integer(1), ctx, error_callback)->repr(), "\"ε\"");
  EXPECT_EQ(codepoints->index(starlark_integer(2), ctx, error_callback)->repr(), "\"ρ\"");

  EXPECT_EQ(codepoint_ords->index(starlark_integer(-3), ctx, error_callback)->repr(), "949");
  EXPECT_EQ(codepoint_ords->index(starlark_integer(-2), ctx, error_callback)->repr(), "953");
  EXPECT_EQ(codepoint_ords->index(starlark_integer(-1), ctx, error_callback)->repr(), "962");
  EXPECT_EQ(codepoint_ords->index(starlark_integer(0), ctx, error_callback)->repr(), "928");
  EXPECT_EQ(codepoint_ords->index(starlark_integer(1), ctx, error_callback)->repr(), "949");
  EXPECT_EQ(codepoint_ords->index(starlark_integer(2), ctx, error_callback)->repr(), "961");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkStringElems, IndexWithBool) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abc"sv);
  auto* elems = str.elems(ctx);

  ASSERT_EQ(nullptr, elems->index(*ctx.true_value(), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: string.elems indices must be integers or slices, not 'bool'");
}

TEST(StarlarkStringElemOrds, IndexWithBool) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abc"sv);
  auto* elems = str.elem_ords(ctx);

  ASSERT_EQ(nullptr, elems->index(*ctx.true_value(), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: string.elem_ords indices must be integers or slices, not 'bool'");
}

TEST(StarlarkStringCodepoints, IndexWithBool) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abc"sv);
  auto* elems = str.codepoints(ctx);

  ASSERT_EQ(nullptr, elems->index(*ctx.true_value(), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: string.codepoints indices must be integers or slices, not 'bool'");
}

TEST(StarlarkStringCodepointOrds, IndexWithBool) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abc"sv);
  auto* elems = str.codepoint_ords(ctx);

  ASSERT_EQ(nullptr, elems->index(*ctx.true_value(), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: string.codepoint_ords indices must be integers or slices, not 'bool'");
}

TEST(StarlarkStringElems, Equals) {
  Arena arena;
  context ctx(arena);

  EXPECT_TRUE(starlark_string(""sv).elems(ctx)->equals(*starlark_string(""sv).elems(ctx)));
  EXPECT_TRUE(starlark_string("a"sv).elems(ctx)->equals(*starlark_string("a"sv).elems(ctx)));
  EXPECT_FALSE(starlark_string(""sv).elems(ctx)->equals(*starlark_string("a"sv).elems(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).elems(ctx)->equals(*starlark_string(""sv).elems(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).elems(ctx)->equals(*starlark_string("b"sv).elems(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).elems(ctx)->equals(*ctx.true_value()));

  EXPECT_TRUE(starlark_string(""sv).elem_ords(ctx)->equals(*starlark_string(""sv).elem_ords(ctx)));
  EXPECT_TRUE(starlark_string("a"sv).elem_ords(ctx)->equals(*starlark_string("a"sv).elem_ords(ctx)));
  EXPECT_FALSE(starlark_string(""sv).elem_ords(ctx)->equals(*starlark_string("a"sv).elem_ords(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).elem_ords(ctx)->equals(*starlark_string(""sv).elem_ords(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).elem_ords(ctx)->equals(*starlark_string("b"sv).elem_ords(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).elem_ords(ctx)->equals(*ctx.true_value()));

  EXPECT_TRUE(starlark_string(""sv).codepoints(ctx)->equals(*starlark_string(""sv).codepoints(ctx)));
  EXPECT_TRUE(starlark_string("a"sv).codepoints(ctx)->equals(*starlark_string("a"sv).codepoints(ctx)));
  EXPECT_FALSE(starlark_string(""sv).codepoints(ctx)->equals(*starlark_string("a"sv).codepoints(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).codepoints(ctx)->equals(*starlark_string(""sv).codepoints(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).codepoints(ctx)->equals(*starlark_string("b"sv).codepoints(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).codepoints(ctx)->equals(*ctx.true_value()));

  EXPECT_TRUE(starlark_string(""sv).codepoint_ords(ctx)->equals(*starlark_string(""sv).codepoint_ords(ctx)));
  EXPECT_TRUE(starlark_string("a"sv).codepoint_ords(ctx)->equals(*starlark_string("a"sv).codepoint_ords(ctx)));
  EXPECT_FALSE(starlark_string(""sv).codepoint_ords(ctx)->equals(*starlark_string("a"sv).codepoint_ords(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).codepoint_ords(ctx)->equals(*starlark_string(""sv).codepoint_ords(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).codepoint_ords(ctx)->equals(*starlark_string("b"sv).codepoint_ords(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).codepoint_ords(ctx)->equals(*ctx.true_value()));


  EXPECT_FALSE(starlark_string(""sv).elems(ctx)->equals(*starlark_string(""sv).elem_ords(ctx)));
  EXPECT_FALSE(starlark_string(""sv).elems(ctx)->equals(*starlark_string(""sv).codepoints(ctx)));
  EXPECT_FALSE(starlark_string(""sv).elems(ctx)->equals(*starlark_string(""sv).codepoint_ords(ctx)));
  EXPECT_FALSE(starlark_string(""sv).elem_ords(ctx)->equals(*starlark_string(""sv).codepoints(ctx)));
  EXPECT_FALSE(starlark_string(""sv).elem_ords(ctx)->equals(*starlark_string(""sv).codepoint_ords(ctx)));
  EXPECT_FALSE(starlark_string(""sv).codepoints(ctx)->equals(*starlark_string(""sv).codepoint_ords(ctx)));
}

TEST(StarlarkStringElems, BinaryIn) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_string(""sv).elems(ctx)->binary_in(starlark_string(""sv), error_callback));
  EXPECT_FALSE(starlark_string("a"sv).elems(ctx)->binary_in(starlark_string(""sv), error_callback));
  EXPECT_TRUE(starlark_string("a"sv).elems(ctx)->binary_in(starlark_string("a"sv), error_callback));
  EXPECT_TRUE(starlark_string("ab"sv).elems(ctx)->binary_in(starlark_string("a"sv), error_callback));
  EXPECT_FALSE(starlark_string("ab"sv).elems(ctx)->binary_in(starlark_string("ab"sv), error_callback));
  EXPECT_FALSE(starlark_string("ab"sv).elems(ctx)->binary_in(starlark_string("x"sv), error_callback));
  EXPECT_TRUE(starlark_string("Περιπτώσεις"sv).elems(ctx)->binary_in(starlark_string("ε"sv), error_callback));
  EXPECT_FALSE(starlark_string("Περιπτώσεις"sv).elems(ctx)->binary_in(starlark_string("Πε"sv), error_callback));

  EXPECT_FALSE(starlark_string(""sv).codepoints(ctx)->binary_in(starlark_string(""sv), error_callback));
  EXPECT_FALSE(starlark_string("a"sv).codepoints(ctx)->binary_in(starlark_string(""sv), error_callback));
  EXPECT_TRUE(starlark_string("a"sv).codepoints(ctx)->binary_in(starlark_string("a"sv), error_callback));
  EXPECT_TRUE(starlark_string("ab"sv).codepoints(ctx)->binary_in(starlark_string("a"sv), error_callback));
  EXPECT_FALSE(starlark_string("ab"sv).codepoints(ctx)->binary_in(starlark_string("ab"sv), error_callback));
  EXPECT_FALSE(starlark_string("ab"sv).codepoints(ctx)->binary_in(starlark_string("x"sv), error_callback));
  EXPECT_TRUE(starlark_string("Περιπτώσεις"sv).codepoints(ctx)->binary_in(starlark_string("ε"sv), error_callback));
  EXPECT_FALSE(starlark_string("Περιπτώσεις"sv).codepoints(ctx)->binary_in(starlark_string("Πε"sv), error_callback));

  EXPECT_TRUE(starlark_string("a"sv).elem_ords(ctx)->binary_in(starlark_integer(97), error_callback));
  EXPECT_TRUE(starlark_string("a"sv).elem_ords(ctx)->binary_in(starlark_bigint(97), error_callback));
  EXPECT_TRUE(starlark_string("ab"sv).elem_ords(ctx)->binary_in(starlark_integer(97), error_callback));
  EXPECT_TRUE(starlark_string("ab"sv).elem_ords(ctx)->binary_in(starlark_bigint(97), error_callback));
  EXPECT_FALSE(starlark_string("ab"sv).elem_ords(ctx)->binary_in(starlark_integer(120), error_callback));
  EXPECT_FALSE(starlark_string("ab"sv).elem_ords(ctx)->binary_in(starlark_bigint(120), error_callback));
  EXPECT_TRUE(starlark_string("Περιπτώσεις"sv).elem_ords(ctx)->binary_in(starlark_integer(949), error_callback));
  EXPECT_TRUE(starlark_string("Περιπτώσεις"sv).elem_ords(ctx)->binary_in(starlark_bigint(949), error_callback));

  EXPECT_TRUE(starlark_string("a"sv).codepoint_ords(ctx)->binary_in(starlark_integer(97), error_callback));
  EXPECT_TRUE(starlark_string("a"sv).codepoint_ords(ctx)->binary_in(starlark_bigint(97), error_callback));
  EXPECT_FALSE(starlark_string("a"sv).codepoint_ords(ctx)->binary_in(starlark_bigint(parse_number("0x10000000000000061", nullptr, 0)), error_callback));
  EXPECT_TRUE(starlark_string("ab"sv).codepoint_ords(ctx)->binary_in(starlark_integer(97), error_callback));
  EXPECT_TRUE(starlark_string("ab"sv).codepoint_ords(ctx)->binary_in(starlark_bigint(97), error_callback));
  EXPECT_FALSE(starlark_string("ab"sv).codepoint_ords(ctx)->binary_in(starlark_integer(120), error_callback));
  EXPECT_FALSE(starlark_string("ab"sv).codepoint_ords(ctx)->binary_in(starlark_bigint(120), error_callback));
  EXPECT_TRUE(starlark_string("Περιπτώσεις"sv).codepoint_ords(ctx)->binary_in(starlark_integer(949), error_callback));
  EXPECT_TRUE(starlark_string("Περιπτώσεις"sv).codepoint_ords(ctx)->binary_in(starlark_bigint(949), error_callback));

  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkStringElems, BinaryInWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_string(""sv).elems(ctx)->binary_in(*ctx.true_value(), error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string.elems>' requires string as left operand, not bool");
}

TEST(StarlarkStringElems, BinaryInWithInt) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_string("a"sv).elems(ctx)->binary_in(starlark_integer(97), error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string.elems>' requires string as left operand, not int");
}

TEST(StarlarkStringElemOrds, BinaryInWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_string(""sv).elem_ords(ctx)->binary_in(*ctx.true_value(), error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string.elem_ords>' requires int as left operand, not bool");
}

TEST(StarlarkStringElemOrds, BinaryInWithString) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_string("a"sv).elem_ords(ctx)->binary_in(starlark_string("a"sv), error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string.elem_ords>' requires int as left operand, not string");
}

TEST(StarlarkStringCodepoints, BinaryInWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_string(""sv).codepoints(ctx)->binary_in(*ctx.true_value(), error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string.codepoints>' requires string as left operand, not bool");
}

TEST(StarlarkStringCodepoints, BinaryInWithInt) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_string("a"sv).codepoints(ctx)->binary_in(starlark_integer(97), error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string.codepoints>' requires string as left operand, not int");
}

TEST(StarlarkStringCodepointOrds, BinaryInWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_string(""sv).codepoint_ords(ctx)->binary_in(*ctx.true_value(), error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string.codepoint_ords>' requires int as left operand, not bool");
}

TEST(StarlarkStringCodepointOrds, BinaryInWithString) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_string("a"sv).codepoint_ords(ctx)->binary_in(starlark_string("a"sv), error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string.codepoint_ords>' requires int as left operand, not string");
}

TEST(StarlarkStringElems, GetIterator) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_string str("abc"sv);
  {
    auto* elems = str.elems(ctx);
    auto* it = elems->get_iterator(true, ctx, error_callback);
    ASSERT_NE(nullptr, it);
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "a");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "b");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "c");
    EXPECT_FALSE(it->has_next());
    it->end_iterator();
  }
  {
    auto* elem_ords = str.elem_ords(ctx);
    auto* it = elem_ords->get_iterator(true, ctx, error_callback);
    ASSERT_NE(nullptr, it);
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "97");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "98");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "99");
    EXPECT_FALSE(it->has_next());
    it->end_iterator();
  }
  {
    auto* codepoints = str.codepoints(ctx);
    auto* it = codepoints->get_iterator(true, ctx, error_callback);
    ASSERT_NE(nullptr, it);
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "a");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "b");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "c");
    EXPECT_FALSE(it->has_next());
    it->end_iterator();
  }
  {
    auto* codepoint_ords = str.codepoint_ords(ctx);
    auto* it = codepoint_ords->get_iterator(true, ctx, error_callback);
    ASSERT_NE(nullptr, it);
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "97");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "98");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "99");
    EXPECT_FALSE(it->has_next());
    it->end_iterator();
  }

  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkStringElems, GetIteratorUnicode) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_string str("Περιπτώσεις"sv);
  {
    auto* elems = str.elems(ctx);
    auto* it = elems->get_iterator(true, ctx, error_callback);
    ASSERT_NE(nullptr, it);
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "Π");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ε");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ρ");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ι");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "π");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "τ");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ώ");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "σ");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ε");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ι");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ς");
    EXPECT_FALSE(it->has_next());
    it->end_iterator();
  }
  {
    auto* elem_ords = str.elem_ords(ctx);
    auto* it = elem_ords->get_iterator(true, ctx, error_callback);
    ASSERT_NE(nullptr, it);
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "928");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "949");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "961");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "953");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "960");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "964");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "974");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "963");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "949");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "953");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "962");
    EXPECT_FALSE(it->has_next());
    it->end_iterator();
  }
  {
    auto* codepoints = str.codepoints(ctx);
    auto* it = codepoints->get_iterator(true, ctx, error_callback);
    ASSERT_NE(nullptr, it);
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "Π");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ε");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ρ");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ι");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "π");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "τ");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ώ");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "σ");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ε");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ι");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->str(), "ς");
    EXPECT_FALSE(it->has_next());
    it->end_iterator();
  }
  {
    auto* codepoint_ords = str.codepoint_ords(ctx);
    auto* it = codepoint_ords->get_iterator(true, ctx, error_callback);
    ASSERT_NE(nullptr, it);
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "928");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "949");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "961");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "953");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "960");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "964");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "974");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "963");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "949");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "953");
    EXPECT_TRUE(it->has_next());
    EXPECT_EQ(it->next()->repr(), "962");
    EXPECT_FALSE(it->has_next());
    it->end_iterator();
  }

  EXPECT_THAT(error_callback.messages, IsEmpty());
}

}  // namespace

