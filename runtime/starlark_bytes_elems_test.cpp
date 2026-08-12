// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string_view>

#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_testing.hpp"

using ::google::protobuf::Arena;
using ::starlark::runtime::context;
using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_bytes;
using ::starlark::runtime::starlark_integer;
using ::starlark::testing::error_handler;
using ::std::literals::string_view_literals::operator""sv;
using ::testing::IsEmpty;
using ::testing::SizeIs;

namespace {

TEST(StarlarkBytesElems, Type) {
  Arena arena;
  context ctx(arena);

  EXPECT_EQ("bytes.elems", starlark_bytes(""sv).elems(ctx)->type());
  EXPECT_EQ("bytes.elem_ords", starlark_bytes(""sv).elem_ords(ctx)->type());
}

TEST(StarlarkBytesElems, Truthy) {
  Arena arena;
  context ctx(arena);

  EXPECT_FALSE(starlark_bytes(""sv).elems(ctx)->truthy());
  EXPECT_TRUE(starlark_bytes("a"sv).elems(ctx)->truthy());
  EXPECT_FALSE(starlark_bytes(""sv).elem_ords(ctx)->truthy());
  EXPECT_TRUE(starlark_bytes("a"sv).elem_ords(ctx)->truthy());
}

TEST(StarlarkBytesElems, Len) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(starlark_bytes(""sv).elems(ctx)->len(true, error_callback), 0);
  EXPECT_EQ(starlark_bytes("a"sv).elems(ctx)->len(true, error_callback), 1);
  EXPECT_EQ(starlark_bytes("ab"sv).elems(ctx)->len(true, error_callback), 2);
  EXPECT_EQ(starlark_bytes(""sv).elem_ords(ctx)->len(true, error_callback), 0);
  EXPECT_EQ(starlark_bytes("a"sv).elem_ords(ctx)->len(true, error_callback), 1);
  EXPECT_EQ(starlark_bytes("ab"sv).elem_ords(ctx)->len(true, error_callback), 2);
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytesElems, Hash) {
  Arena arena;
  context ctx(arena);

  EXPECT_EQ(starlark_bytes(""sv).elems(ctx)->hash(), -1);
  EXPECT_EQ(starlark_bytes(""sv).elem_ords(ctx)->hash(), -1);
}

TEST(StarlarkBytesElems, Str) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(starlark_bytes("abc"sv).elems(ctx)->str(), "b\"abc\".elems()");
  EXPECT_EQ(starlark_bytes("abc"sv).elem_ords(ctx)->str(), "b\"abc\".elem_ords()");
}

TEST(StarlarkBytesElems, SliceRange) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(starlark_bytes("abc"sv).elems(ctx)->slice_range(*ctx.one(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback)->str(), "b\"bc\".elems()");
  EXPECT_EQ(starlark_bytes("abc"sv).elems(ctx)->slice_range(*ctx.none_value(), *ctx.one(), *ctx.none_value(), ctx, error_callback)->str(), "b\"a\".elems()");
  EXPECT_EQ(starlark_bytes("abc"sv).elems(ctx)->slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.minus_one(), ctx, error_callback)->str(), "b\"cba\".elems()");
  EXPECT_EQ(starlark_bytes("abc"sv).elem_ords(ctx)->slice_range(*ctx.one(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback)->str(), "b\"bc\".elem_ords()");
  EXPECT_EQ(starlark_bytes("abc"sv).elem_ords(ctx)->slice_range(*ctx.none_value(), *ctx.one(), *ctx.none_value(), ctx, error_callback)->str(), "b\"a\".elem_ords()");
  EXPECT_EQ(starlark_bytes("abc"sv).elem_ords(ctx)->slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.minus_one(), ctx, error_callback)->str(), "b\"cba\".elem_ords()");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytesElems, SliceRangeWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(nullptr, starlark_bytes("abc"sv).elems(ctx)->slice_range(*ctx.true_value(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "slice indices must be integers, not 'bool'");
}

TEST(StarlarkBytesElemOrds, SliceRangeWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ(nullptr, starlark_bytes("abc"sv).elem_ords(ctx)->slice_range(*ctx.true_value(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "slice indices must be integers, not 'bool'");
}

TEST(StarlarkBytesElems, Index) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  auto* elems = bytes.elems(ctx);
  auto* elem_ords = bytes.elem_ords(ctx);

  EXPECT_EQ(elems->index(starlark_integer(-3), ctx, error_callback)->repr(), "b\"a\"");
  EXPECT_EQ(elems->index(starlark_integer(-2), ctx, error_callback)->repr(), "b\"b\"");
  EXPECT_EQ(elems->index(*ctx.minus_one(), ctx, error_callback)->repr(), "b\"c\"");
  EXPECT_EQ(elems->index(*ctx.zero(), ctx, error_callback)->repr(), "b\"a\"");
  EXPECT_EQ(elems->index(*ctx.one(), ctx, error_callback)->repr(), "b\"b\"");
  EXPECT_EQ(elems->index(starlark_integer(2), ctx, error_callback)->repr(), "b\"c\"");

  EXPECT_EQ(elem_ords->index(starlark_integer(-3), ctx, error_callback)->repr(), "97");
  EXPECT_EQ(elem_ords->index(starlark_integer(-2), ctx, error_callback)->repr(), "98");
  EXPECT_EQ(elem_ords->index(*ctx.minus_one(), ctx, error_callback)->repr(), "99");
  EXPECT_EQ(elem_ords->index(*ctx.zero(), ctx, error_callback)->repr(), "97");
  EXPECT_EQ(elem_ords->index(*ctx.one(), ctx, error_callback)->repr(), "98");
  EXPECT_EQ(elem_ords->index(starlark_integer(2), ctx, error_callback)->repr(), "99");

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
  EXPECT_EQ(error_callback.messages[0], "bytes.elems indices must be integers or slices, not 'bool'");
}

TEST(StarlarkBytesElemOrds, IndexWithBool) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_bytes bytes("abc"sv);
  auto* elems = bytes.elem_ords(ctx);

  ASSERT_EQ(nullptr, elems->index(*ctx.true_value(), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "bytes.elem_ords indices must be integers or slices, not 'bool'");
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

  EXPECT_TRUE(starlark_bytes(""sv).elem_ords(ctx)->equals(*starlark_bytes(""sv).elem_ords(ctx)));
  EXPECT_TRUE(starlark_bytes("a"sv).elem_ords(ctx)->equals(*starlark_bytes("a"sv).elem_ords(ctx)));
  EXPECT_FALSE(starlark_bytes(""sv).elem_ords(ctx)->equals(*starlark_bytes("a"sv).elem_ords(ctx)));
  EXPECT_FALSE(starlark_bytes("a"sv).elem_ords(ctx)->equals(*starlark_bytes(""sv).elem_ords(ctx)));
  EXPECT_FALSE(starlark_bytes("a"sv).elem_ords(ctx)->equals(*starlark_bytes("b"sv).elem_ords(ctx)));
  EXPECT_FALSE(starlark_bytes("a"sv).elem_ords(ctx)->equals(*ctx.true_value()));

  EXPECT_FALSE(starlark_bytes(""sv).elems(ctx)->equals(*starlark_bytes(""sv).elem_ords(ctx)));
}

TEST(StarlarkBytesElems, BinaryIn) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_bytes(""sv).elems(ctx)->binary_in(starlark_bytes(""sv), error_callback));
  EXPECT_FALSE(starlark_bytes("a"sv).elems(ctx)->binary_in(starlark_bytes(""sv), error_callback));
  EXPECT_TRUE(starlark_bytes("a"sv).elems(ctx)->binary_in(starlark_bytes("a"sv), error_callback));
  EXPECT_TRUE(starlark_bytes("a"sv).elems(ctx)->binary_in(starlark_integer(97), error_callback));
  EXPECT_TRUE(starlark_bytes("a"sv).elems(ctx)->binary_in(starlark_bigint(97), error_callback));
  EXPECT_TRUE(starlark_bytes("ab"sv).elems(ctx)->binary_in(starlark_bytes("a"sv), error_callback));
  EXPECT_FALSE(starlark_bytes("ab"sv).elems(ctx)->binary_in(starlark_bytes("ab"sv), error_callback));
  EXPECT_FALSE(starlark_bytes("ab"sv).elems(ctx)->binary_in(starlark_bytes("x"sv), error_callback));

  EXPECT_FALSE(starlark_bytes(""sv).elem_ords(ctx)->binary_in(starlark_bytes(""sv), error_callback));
  EXPECT_FALSE(starlark_bytes("a"sv).elem_ords(ctx)->binary_in(starlark_bytes(""sv), error_callback));
  EXPECT_TRUE(starlark_bytes("a"sv).elem_ords(ctx)->binary_in(starlark_bytes("a"sv), error_callback));
  EXPECT_TRUE(starlark_bytes("a"sv).elem_ords(ctx)->binary_in(starlark_integer(97), error_callback));
  EXPECT_TRUE(starlark_bytes("a"sv).elem_ords(ctx)->binary_in(starlark_bigint(97), error_callback));
  EXPECT_TRUE(starlark_bytes("ab"sv).elem_ords(ctx)->binary_in(starlark_bytes("a"sv), error_callback));
  EXPECT_FALSE(starlark_bytes("ab"sv).elem_ords(ctx)->binary_in(starlark_bytes("ab"sv), error_callback));
  EXPECT_FALSE(starlark_bytes("ab"sv).elem_ords(ctx)->binary_in(starlark_bytes("x"sv), error_callback));

  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytesElems, BinaryInWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_bytes(""sv).elems(ctx)->binary_in(*ctx.true_value(), error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "argument should be integer or bytes object, not 'bool'");
}

TEST(StarlarkBytesElemOrds, BinaryInWithBoolean) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_FALSE(starlark_bytes(""sv).elem_ords(ctx)->binary_in(*ctx.true_value(), error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "argument should be integer or bytes object, not 'bool'");
}

TEST(StarlarkBytesElems, GetIterator) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_bytes bytes("abc"sv);
  {
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
  }
  {
    auto* elem_ords = bytes.elem_ords(ctx);
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
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

}  // namespace

