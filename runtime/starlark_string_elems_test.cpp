// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string_view>

#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_testing.hpp"

using ::google::protobuf::Arena;
using ::starlark::runtime::context;
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
}

TEST(StarlarkStringElems, Truthy) {
  Arena arena;
  context ctx(arena);

  EXPECT_FALSE(starlark_string(""sv).elems(ctx)->truthy());
  EXPECT_TRUE(starlark_string("a"sv).elems(ctx)->truthy());
}

TEST(StarlarkStringElems, Len) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(starlark_string(""sv).elems(ctx)->len(true, error_callback), 0);
  EXPECT_EQ(starlark_string("a"sv).elems(ctx)->len(true, error_callback), 1);
  EXPECT_EQ(starlark_string("ab"sv).elems(ctx)->len(true, error_callback), 2);
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).elems(ctx)->len(true, error_callback), 11);
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkStringElems, Hash) {
  Arena arena;
  context ctx(arena);

  EXPECT_EQ(starlark_string(""sv).elems(ctx)->hash(), -1);
}

TEST(StarlarkStringElems, Str) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(starlark_string("abc"sv).elems(ctx)->str(), "\"abc\".elems()");
  EXPECT_EQ(starlark_string("Περιπτώσεις"sv).elems(ctx)->str(), "\"Περιπτώσεις\".elems()");
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

TEST(StarlarkStringElems, Index) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  auto* elems = str.elems(ctx);

  EXPECT_EQ(elems->index(starlark_integer(-3), ctx, error_callback)->repr(), "\"a\"");
  EXPECT_EQ(elems->index(starlark_integer(-2), ctx, error_callback)->repr(), "\"b\"");
  EXPECT_EQ(elems->index(starlark_integer(-1), ctx, error_callback)->repr(), "\"c\"");
  EXPECT_EQ(elems->index(starlark_integer(0), ctx, error_callback)->repr(), "\"a\"");
  EXPECT_EQ(elems->index(starlark_integer(1), ctx, error_callback)->repr(), "\"b\"");
  EXPECT_EQ(elems->index(starlark_integer(2), ctx, error_callback)->repr(), "\"c\"");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkStringElems, IndexUnicode) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("Περιπτώσεις"sv);
  auto* elems = str.elems(ctx);

  EXPECT_EQ(elems->index(starlark_integer(-3), ctx, error_callback)->repr(), "\"ε\"");
  EXPECT_EQ(elems->index(starlark_integer(-2), ctx, error_callback)->repr(), "\"ι\"");
  EXPECT_EQ(elems->index(starlark_integer(-1), ctx, error_callback)->repr(), "\"ς\"");
  EXPECT_EQ(elems->index(starlark_integer(0), ctx, error_callback)->repr(), "\"Π\"");
  EXPECT_EQ(elems->index(starlark_integer(1), ctx, error_callback)->repr(), "\"ε\"");
  EXPECT_EQ(elems->index(starlark_integer(2), ctx, error_callback)->repr(), "\"ρ\"");
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

TEST(StarlarkStringElems, Equals) {
  Arena arena;
  context ctx(arena);

  EXPECT_TRUE(starlark_string(""sv).elems(ctx)->equals(*starlark_string(""sv).elems(ctx)));
  EXPECT_TRUE(starlark_string("a"sv).elems(ctx)->equals(*starlark_string("a"sv).elems(ctx)));
  EXPECT_FALSE(starlark_string(""sv).elems(ctx)->equals(*starlark_string("a"sv).elems(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).elems(ctx)->equals(*starlark_string(""sv).elems(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).elems(ctx)->equals(*starlark_string("b"sv).elems(ctx)));
  EXPECT_FALSE(starlark_string("a"sv).elems(ctx)->equals(*ctx.true_value()));
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

TEST(StarlarkStringElems, GetIterator) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_string str("abc"sv);
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
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkStringElems, GetIteratorUnicode) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_string str("Περιπτώσεις"sv);
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
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

}  // namespace

