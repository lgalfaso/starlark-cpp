// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string_view>

#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_testing.hpp"

using ::google::protobuf::Arena;
using ::starlark::runtime::context;
using ::starlark::testing::error_handler;
using ::starlark::runtime::starlark_bytes;
using ::starlark::runtime::starlark_integer;
using ::std::literals::string_view_literals::operator""sv;
using ::testing::IsEmpty;
using ::testing::SizeIs;

namespace {

TEST(StarlarkBytesElems, Type) {
  Arena arena;
  context ctx(arena);

  EXPECT_EQ("bytes.elems", starlark_bytes(""sv).elems(ctx)->type());
}

TEST(StarlarkBytesElems, Truthy) {
  Arena arena;
  context ctx(arena);

  EXPECT_FALSE(starlark_bytes(""sv).elems(ctx)->truthy());
  EXPECT_TRUE(starlark_bytes("a"sv).elems(ctx)->truthy());
}

TEST(StarlarkBytesElems, Len) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(starlark_bytes(""sv).elems(ctx)->len(true, error_callback), 0);
  EXPECT_EQ(starlark_bytes("a"sv).elems(ctx)->len(true, error_callback), 1);
  EXPECT_EQ(starlark_bytes("ab"sv).elems(ctx)->len(true, error_callback), 2);
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytesElems, Hash) {
  Arena arena;
  context ctx(arena);

  EXPECT_EQ(starlark_bytes(""sv).elems(ctx)->hash(), -1);
}

TEST(StarlarkBytesElems, Str) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(starlark_bytes("abc"sv).elems(ctx)->str(), "b\"abc\".elems()");
}

TEST(StarlarkBytesElems, SliceRange) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(starlark_bytes("abc"sv).elems(ctx)->slice_range(*ctx.one(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback)->str(), "b\"bc\".elems()");
  EXPECT_EQ(starlark_bytes("abc"sv).elems(ctx)->slice_range(*ctx.none_value(), *ctx.one(), *ctx.none_value(), ctx, error_callback)->str(), "b\"a\".elems()");
  EXPECT_EQ(starlark_bytes("abc"sv).elems(ctx)->slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.minus_one(), ctx, error_callback)->str(), "b\"cba\".elems()");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytesElems, SliceRangeWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(nullptr, starlark_bytes("abc"sv).elems(ctx)->slice_range(*ctx.true_value(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
}

TEST(StarlarkBytesElems, Index) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  auto* elems = bytes.elems(ctx);

  EXPECT_EQ(elems->index(starlark_integer(-3), ctx, error_callback)->repr(), "b\"a\"");
  EXPECT_EQ(elems->index(starlark_integer(-2), ctx, error_callback)->repr(), "b\"b\"");
  EXPECT_EQ(elems->index(starlark_integer(-1), ctx, error_callback)->repr(), "b\"c\"");
  EXPECT_EQ(elems->index(starlark_integer(0), ctx, error_callback)->repr(), "b\"a\"");
  EXPECT_EQ(elems->index(starlark_integer(1), ctx, error_callback)->repr(), "b\"b\"");
  EXPECT_EQ(elems->index(starlark_integer(2), ctx, error_callback)->repr(), "b\"c\"");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytesElems, IndexWithBool) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_bytes bytes("abc"sv);
  auto* elems = bytes.elems(ctx);

  ASSERT_EQ(nullptr, elems->index(*ctx.true_value(), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bytes.elems indices must be integers or slices, not 'bool'");
}

TEST(StarlarkBytesElems, Equals) {
  Arena arena;
  context ctx(arena);

  EXPECT_TRUE(starlark_bytes(""sv).elems(ctx)->equals(*starlark_bytes(""sv).elems(ctx)));
  EXPECT_TRUE(starlark_bytes("a"sv).elems(ctx)->equals(*starlark_bytes("a"sv).elems(ctx)));
  EXPECT_FALSE(starlark_bytes(""sv).elems(ctx)->equals(*starlark_bytes("a"sv).elems(ctx)));
  EXPECT_FALSE(starlark_bytes("a"sv).elems(ctx)->equals(*starlark_bytes(""sv).elems(ctx)));
  EXPECT_FALSE(starlark_bytes("a"sv).elems(ctx)->equals(*starlark_bytes("b"sv).elems(ctx)));
  EXPECT_FALSE(starlark_bytes("a"sv).elems(ctx)->equals(*ctx.true_value()));
}

TEST(StarlarkBytesElems, BinaryIn) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_bytes(""sv).elems(ctx)->binary_in(starlark_bytes(""sv), error_callback));
  EXPECT_FALSE(starlark_bytes("a"sv).elems(ctx)->binary_in(starlark_bytes(""sv), error_callback));
  EXPECT_TRUE(starlark_bytes("a"sv).elems(ctx)->binary_in(starlark_bytes("a"sv), error_callback));
  EXPECT_TRUE(starlark_bytes("ab"sv).elems(ctx)->binary_in(starlark_bytes("a"sv), error_callback));
  EXPECT_FALSE(starlark_bytes("ab"sv).elems(ctx)->binary_in(starlark_bytes("ab"sv), error_callback));
  EXPECT_FALSE(starlark_bytes("ab"sv).elems(ctx)->binary_in(starlark_bytes("x"sv), error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytesElems, BinaryInWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_bytes(""sv).elems(ctx)->binary_in(*ctx.true_value(), error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'bool'");
}

TEST(StarlarkBytesElems, GetIterator) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_bytes bytes("abc"sv);
  auto* elems = bytes.elems(ctx);
  auto* it = elems->get_iterator(true, ctx, error_callback);
  ASSERT_NE(nullptr, it);
  EXPECT_TRUE(it->has_next());
  EXPECT_EQ(it->next()->str(), "b\"a\"");
  EXPECT_TRUE(it->has_next());
  EXPECT_EQ(it->next()->str(), "b\"b\"");
  EXPECT_TRUE(it->has_next());
  EXPECT_EQ(it->next()->str(), "b\"c\"");
  EXPECT_FALSE(it->has_next());
  it->end_iterator();
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

}  // namespace

