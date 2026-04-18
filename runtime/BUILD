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
    name = "levenshtein",
    srcs = ["levenshtein.cpp"],
    hdrs = ["levenshtein.hpp"],
)

cc_test(
    name = "levenshtein_test",
    size = "small",
    srcs = ["levenshtein_test.cpp"],
    deps = [
        ":levenshtein",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "options",
    srcs = ["options.cpp"],
    hdrs = ["options.hpp"],
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
    name = "error_messages",
    srcs = ["error_messages.cpp"],
    hdrs = ["error_messages.hpp"],
)

cc_library(
    name = "starlark_testing",
    testonly = True,
    hdrs = ["starlark_testing.hpp"],
    srcs = ["starlark_testing.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":error_fn",
    ],
)

cc_library(
    name = "starlark_types",
    srcs = ["starlark_types.cpp"],
    hdrs = ["starlark_types.hpp"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "starlark_object",
    srcs = ["starlark_object.cpp"],
    hdrs = ["starlark_object.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":error_fn",
        ":error_messages",
        ":levenshtein",
        ":starlark_types",
        "//bigint:number",
        "//containers:linked_hash_map",
        "//status_or:status",
        "@protobuf",
    ],
)

cc_library(
    name = "starlark_object_impl",
    srcs = ["starlark_object_impl.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":starlark_bool",
        ":starlark_bytes",
        ":starlark_integer",
        ":starlark_none",
        ":starlark_numeric",
        ":starlark_object",
        ":starlark_string",
        "@protobuf",
    ],
)

cc_library(
    name = "starlark_bool",
    srcs = ["starlark_bool.cpp"],
    hdrs = ["starlark_bool.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":starlark_object",
        ":starlark_types",
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
        ":starlark_function_builder_impl",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_none",
    srcs = ["starlark_none.cpp"],
    hdrs = ["starlark_none.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":starlark_object",
        ":starlark_types",
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
        ":starlark_function_builder_impl",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
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
        ":starlark_types",
        "//bigint:number",
    ],
)

cc_library(
    name = "starlark_numeric_impl",
    srcs = ["starlark_numeric_impl.cpp"],
    visibility = ["//visibility:public"],
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
        ":starlark_function_builder_impl",
        ":starlark_numeric",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_testing",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_bigint",
    srcs = ["starlark_bigint.cpp"],
    hdrs = ["starlark_bigint.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":error_messages",
        ":hash",
        ":options",
        ":starlark_float",
        ":starlark_numeric",
        ":starlark_object",
        ":starlark_types",
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
        ":starlark_bytes",
        ":starlark_dictionary",
        ":starlark_float",
        ":starlark_function",
        ":starlark_function_builder_impl",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_bytes",
    srcs = ["starlark_bytes.cpp"],
    hdrs = ["starlark_bytes.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":error_messages",
        ":hex_encoder",
        ":options",
        ":siphash",
        ":starlark_numeric",
        ":starlark_object",
        ":starlark_range",
        ":starlark_tuple",
        ":starlark_types",
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
        ":starlark_function_builder_impl",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "starlark_bytes_elems_test",
    size = "small",
    srcs = ["starlark_bytes_elems_test.cpp"],
    deps = [
        ":starlark_bytes",
        ":starlark_function_builder_impl",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_testing",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_dictionary",
    srcs = ["starlark_dictionary.cpp"],
    hdrs = ["starlark_dictionary.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":error_fn",
        ":error_messages",
        ":starlark_list",
        ":starlark_none",
        ":starlark_object",
        ":starlark_string",
        ":starlark_tuple",
        ":starlark_types",
        "//containers:linked_hash_map",
        "//status_or:status",
    ],
)

cc_test(
    name = "starlark_dictionary_test",
    size = "small",
    srcs = ["starlark_dictionary_test.cpp"],
    deps = [
        ":starlark_bool",
        ":starlark_dictionary",
        ":starlark_function_builder_impl",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_string",
        ":starlark_testing",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_integer",
    srcs = ["starlark_integer.cpp"],
    hdrs = ["starlark_integer.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":error_messages",
        ":hash",
        ":options",
        ":starlark_bigint",
        ":starlark_numeric",
        ":starlark_object",
        ":starlark_types",
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
        ":starlark_bytes",
        ":starlark_dictionary",
        ":starlark_float",
        ":starlark_function",
        ":starlark_function_builder_impl",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "starlark_integer_overflow_test",
    size = "medium",
    srcs = ["starlark_integer_overflow_test.cpp"],
    deps = [
        ":starlark_function_builder_impl",
        ":starlark_integer",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_testing",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_float",
    srcs = ["starlark_float.cpp"],
    hdrs = ["starlark_float.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":error_messages",
        ":hash",
        ":starlark_numeric",
        ":starlark_object",
        ":starlark_types",
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
        ":starlark_function_builder_impl",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_list",
    srcs = ["starlark_list.cpp"],
    hdrs = ["starlark_list.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":error_messages",
        ":starlark_integer",
        ":starlark_object",
        ":starlark_types",
        "//status_or:status",
    ],
)

cc_test(
    name = "starlark_list_test",
    size = "small",
    srcs = ["starlark_list_test.cpp"],
    deps = [
        ":starlark_bool",
        ":starlark_function_builder_impl",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
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
        ":starlark_numeric",
        ":starlark_object",
        ":starlark_types",
    ],
)

cc_test(
    name = "starlark_range_test",
    size = "small",
    srcs = ["starlark_range_test.cpp"],
    deps = [
        ":starlark_function_builder_impl",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_range",
        ":starlark_testing",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "starlark_range_slice_test",
    size = "small",
    srcs = ["starlark_range_slice_test.cpp"],
    deps = [
        ":starlark_function_builder_impl",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_range",
        ":starlark_testing",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_set",
    srcs = ["starlark_set.cpp"],
    hdrs = ["starlark_set.hpp"],
    deps = [
        ":error_messages",
        ":starlark_object",
        ":starlark_types",
        "//containers:linked_hash_set",
        "//status_or:status",
    ],
)

cc_test(
    name = "starlark_set_test",
    size = "small",
    srcs = ["starlark_set_test.cpp"],
    deps = [
        ":starlark_bool",
        ":starlark_function_builder_impl",
        ":starlark_integer",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
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
    visibility = ["//visibility:public"],
    deps = [
        ":error_messages",
        ":hex_encoder",
        ":options",
        ":siphash",
        ":starlark_numeric",
        ":starlark_object",
        ":starlark_range",
        ":starlark_tuple",
        ":starlark_types",
        "//bigint:number",
        "//unicode:utf8_reader",
        "//unicode:utf8_reverse_reader",
        "//unicode:word_break",
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
        ":starlark_function_builder_impl",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "starlark_string_elems_test",
    size = "small",
    srcs = ["starlark_string_elems_test.cpp"],
    deps = [
        ":starlark_function_builder_impl",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_string",
        ":starlark_testing",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_tuple",
    srcs = ["starlark_tuple.cpp"],
    hdrs = ["starlark_tuple.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":error_messages",
        ":starlark_list",
        ":starlark_object",
        ":starlark_types",
    ],
)

cc_test(
    name = "starlark_tuple_test",
    size = "small",
    srcs = ["starlark_tuple_test.cpp"],
    deps = [
        ":starlark_bool",
        ":starlark_function_builder_impl",
        ":starlark_integer",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_function",
    srcs = ["starlark_function.cpp"],
    hdrs = ["starlark_function.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":error_messages",
        ":starlark_bool",
        ":starlark_bytes",
        ":starlark_dictionary",
        ":starlark_list",
        ":starlark_numeric",
        ":starlark_object",
        ":starlark_range",
        ":starlark_set",
        ":starlark_string",
        ":starlark_tuple",
        ":starlark_types",
        "//unicode:encode",
        "//unicode:utf8_reader",
    ],
)

cc_test(
    name = "starlark_function_test",
    size = "small",
    srcs = ["starlark_function_test.cpp"],
    deps = [
        ":starlark_bytes",
        ":starlark_function",
        ":starlark_function_builder_impl",
        ":starlark_integer",
        ":starlark_list",
        ":starlark_none",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        ":starlark_string",
        ":starlark_testing",
        ":starlark_tuple",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "starlark_function_builder_impl",
    srcs = ["starlark_function_builder_impl.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":starlark_function",
    ],
)


cc_test(
    name = "float_hash_test",
    size = "small",
    srcs = ["float_hash_test.cpp"],
    deps = [
        ":starlark_float",
        ":starlark_function_builder_impl",
        ":starlark_numeric_impl",
        ":starlark_object_impl",
        "@googletest//:gtest_main",
    ],
)
