# Copyright 2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "hash",
    hdrs = ["hash.hpp"],
)

cc_library(
    name = "siphash",
    srcs = ["siphash.cpp"],
    hdrs = ["siphash.hpp"],
)

cc_test(
    name = "siphash_test",
    size = "small",
    srcs = ["siphash_test.cpp"],
    deps = [
        ":siphash",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_object",
    srcs = ["starlark_object.cpp"],
    hdrs = ["starlark_object.hpp"],
)

cc_library(
    name = "starlark_bool",
    srcs = ["starlark_bool.cpp"],
    hdrs = ["starlark_bool.hpp"],
    deps = [
        ":starlark_object",
    ],
)

cc_test(
    name = "starlark_bool_test",
    size = "small",
    srcs = ["starlark_bool_test.cpp"],
    deps = [
        ":starlark_bigint",
        ":starlark_bool",
        ":starlark_integer",
        ":starlark_none",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_none",
    srcs = ["starlark_none.cpp"],
    hdrs = ["starlark_none.hpp"],
    deps = [
        ":starlark_object",
    ],
)

cc_test(
    name = "starlark_none_test",
    size = "small",
    srcs = ["starlark_none_test.cpp"],
    deps = [
        ":starlark_bigint",
        ":starlark_bool",
        ":starlark_integer",
        ":starlark_none",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_numeric",
    srcs = ["starlark_numeric.cpp"],
    hdrs = ["starlark_numeric.hpp"],
    deps = [
        ":starlark_object",
        "//bigint:number",
    ],
)

cc_library(
    name = "starlark_bigint",
    srcs = ["starlark_bigint.cpp"],
    hdrs = ["starlark_bigint.hpp"],
    deps = [
        ":hash",
        ":starlark_numeric",
        ":starlark_object",
        "//bigint:number",
    ],
)

cc_test(
    name = "starlark_bigint_test",
    size = "small",
    srcs = ["starlark_bigint_test.cpp"],
    deps = [
        ":starlark_bigint",
        ":starlark_bool",
        ":starlark_integer",
        ":starlark_none",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_integer",
    srcs = ["starlark_integer.cpp"],
    hdrs = ["starlark_integer.hpp"],
    deps = [
        ":hash",
        ":starlark_numeric",
        ":starlark_object",
        "//bigint:number",
    ],
)

cc_test(
    name = "starlark_integer_test",
    size = "small",
    srcs = ["starlark_integer_test.cpp"],
    deps = [
        ":starlark_bigint",
        ":starlark_bool",
        ":starlark_integer",
        ":starlark_none",
        "@googletest//:gtest_main",
    ],
)

