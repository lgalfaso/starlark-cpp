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
    name = "starlark_bytes",
    srcs = ["starlark_bytes.cpp"],
    hdrs = ["starlark_bytes.hpp"],
    deps = [
        ":siphash",
        ":starlark_object",
    ],
)

cc_test(
    name = "starlark_bytes_test",
    size = "small",
    srcs = ["starlark_bytes_test.cpp"],
    deps = [
        ":starlark_bigint",
        ":starlark_bool",
        ":starlark_bytes",
        ":starlark_float",
        ":starlark_integer",
        ":starlark_none",
        ":starlark_string",
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

cc_library(
    name = "starlark_float",
    srcs = ["starlark_float.cpp"],
    hdrs = ["starlark_float.hpp"],
    deps = [
        ":hash",
        ":starlark_numeric",
        ":starlark_object",
    ],
)

cc_test(
    name = "starlark_float_test",
    size = "small",
    srcs = ["starlark_float_test.cpp"],
    deps = [
        ":starlark_bigint",
        ":starlark_bool",
        ":starlark_bytes",
        ":starlark_dictionary",
        ":starlark_float",
        ":starlark_function",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_struct",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_string",
    srcs = ["starlark_string.cpp"],
    hdrs = ["starlark_string.hpp"],
    deps = [
        ":siphash",
        ":starlark_object",
        "//unicode:encode",
        "//unicode:utf8_reader",
    ],
)

cc_test(
    name = "starlark_string_test",
    size = "small",
    srcs = ["starlark_string_test.cpp"],
    deps = [
        ":starlark_bigint",
        ":starlark_bool",
        ":starlark_bytes",
        ":starlark_float",
        ":starlark_integer",
        ":starlark_none",
        ":starlark_string",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "float_hash_test",
    size = "small",
    srcs = ["float_hash_test.cpp"],
    deps = [
        ":starlark_float",
        "@googletest//:gtest_main",
    ],
)

