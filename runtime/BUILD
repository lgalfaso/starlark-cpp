# Copyright 2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "hash",
    hdrs = ["hash.hpp"],
)

cc_library(
    name = "hex_encoder",
    srcs = ["hex_encoder.cpp"],
    hdrs = ["hex_encoder.hpp"],
    deps = [
        "//unicode:encode",
    ],
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
    name = "error_fn",
    hdrs = ["error_fn.hpp"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "starlark_testing",
    hdrs = ["starlark_testing.hpp"],
    visibility = ["//visibility:public"],
    testonly = True,
    deps = [
        ":error_fn",
    ],
)

cc_library(
    name = "starlark_object",
    srcs = ["starlark_object.cpp"],
    hdrs = ["starlark_object.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":error_fn",
        "@protobuf//:protobuf",
    ],
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
        ":starlark_bytes",
        ":starlark_dictionary",
        ":starlark_float",
        ":starlark_function",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_struct",
        ":starlark_testing",
        ":starlark_tuple",
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
        ":starlark_bytes",
        ":starlark_dictionary",
        ":starlark_float",
        ":starlark_function",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_struct",
        ":starlark_tuple",
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
    name = "starlark_numeric_impl",
    srcs = ["starlark_numeric_impl.cpp"],
    deps = [
        ":starlark_bigint",
        ":starlark_float",
        ":starlark_integer",
        ":starlark_numeric",
        "//bigint:number",
    ],
)

cc_test(
    name = "starlark_numeric_test",
    size = "small",
    srcs = ["starlark_numeric_test.cpp"],
    deps = [
        ":starlark_numeric",
        ":starlark_numeric_impl",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_bigint",
    srcs = ["starlark_bigint.cpp"],
    hdrs = ["starlark_bigint.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":hash",
        ":starlark_float",
        ":starlark_numeric",
        ":starlark_object",
        "//bigint:number",
        "//grammar:options",
    ],
)

cc_test(
    name = "starlark_bigint_test",
    size = "small",
    srcs = ["starlark_bigint_test.cpp"],
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
        ":starlark_numeric_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_struct",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_bytes",
    srcs = ["starlark_bytes.cpp"],
    hdrs = ["starlark_bytes.hpp"],
    deps = [
        ":hex_encoder",
        ":siphash",
        ":starlark_numeric",
        ":starlark_object",
        "//grammar:options",
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
        ":starlark_dictionary",
        ":starlark_float",
        ":starlark_function",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_struct",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_dictionary",
    srcs = ["starlark_dictionary.cpp"],
    hdrs = ["starlark_dictionary.hpp"],
    deps = [
        ":error_fn",
        ":starlark_object",
        "//containers:linked_hash_map",
    ],
)

cc_test(
    name = "starlark_dictionary_test",
    size = "small",
    srcs = ["starlark_dictionary_test.cpp"],
    deps = [
        ":starlark_bool",
        ":starlark_dictionary",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_string",
        ":starlark_testing",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_integer",
    srcs = ["starlark_integer.cpp"],
    hdrs = ["starlark_integer.hpp"],
    deps = [
        ":hash",
        ":starlark_bigint",
        ":starlark_numeric",
        ":starlark_object",
        "//bigint:number",
        "//grammar:options",
    ],
)

cc_test(
    name = "starlark_integer_test",
    size = "small",
    srcs = ["starlark_integer_test.cpp"],
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
        ":starlark_numeric_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_struct",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "starlark_integer_overflow_test",
    size = "small",
    srcs = ["starlark_integer_overflow_test.cpp"],
    deps = [
        ":starlark_integer",
        ":starlark_numeric_impl",
        ":starlark_testing",
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
        ":starlark_numeric_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_struct",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_list",
    srcs = ["starlark_list.cpp"],
    hdrs = ["starlark_list.hpp"],
    deps = [
        ":starlark_object",
        ":starlark_integer",
    ],
)

cc_test(
    name = "starlark_list_test",
    size = "small",
    srcs = ["starlark_list_test.cpp"],
    deps = [
        ":starlark_bool",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_range",
    srcs = ["starlark_range.cpp"],
    hdrs = ["starlark_range.hpp"],
    deps = [
        ":starlark_object",
    ],
)

cc_test(
    name = "starlark_range_test",
    size = "small",
    srcs = ["starlark_range_test.cpp"],
    deps = [
        ":starlark_range",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_set",
    srcs = ["starlark_set.cpp"],
    hdrs = ["starlark_set.hpp"],
    deps = [
        ":starlark_object",
        "//containers:linked_hash_set",
    ],
)

cc_test(
    name = "starlark_set_test",
    size = "small",
    srcs = ["starlark_set_test.cpp"],
    deps = [
        ":starlark_bool",
        ":starlark_integer",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_set",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_string",
    srcs = ["starlark_string.cpp"],
    hdrs = ["starlark_string.hpp"],
    deps = [
        ":hex_encoder",
        ":siphash",
        ":starlark_numeric",
        ":starlark_object",
        "//bigint:number",
        "//grammar:options",
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
        ":starlark_dictionary",
        ":starlark_float",
        ":starlark_function",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_struct",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_struct",
    srcs = ["starlark_struct.cpp"],
    hdrs = ["starlark_struct.hpp"],
    deps = [
        ":starlark_object",
    ],
)

cc_test(
    name = "starlark_struct_test",
    size = "small",
    srcs = ["starlark_struct_test.cpp"],
    deps = [
        ":starlark_struct",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_tuple",
    srcs = ["starlark_tuple.cpp"],
    hdrs = ["starlark_tuple.hpp"],
    deps = [
        ":starlark_list",
        ":starlark_object",
    ],
)

cc_test(
    name = "starlark_tuple_test",
    size = "small",
    srcs = ["starlark_tuple_test.cpp"],
    deps = [
        ":starlark_bool",
        ":starlark_integer",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_function",
    srcs = ["starlark_function.cpp"],
    hdrs = ["starlark_function.hpp"],
    deps = [
        ":starlark_object",
    ],
)

cc_test(
    name = "starlark_function_test",
    size = "small",
    srcs = ["starlark_function_test.cpp"],
    deps = [
        ":starlark_function",
        ":starlark_testing",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "float_hash_test",
    size = "small",
    srcs = ["float_hash_test.cpp"],
    deps = [
        ":starlark_float",
        ":starlark_numeric_impl",
        "@googletest//:gtest_main",
    ],
)
