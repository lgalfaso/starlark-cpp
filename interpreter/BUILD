# Copyright 2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "interpreter",
    srcs = ["interpreter.cpp"],
    hdrs = ["interpreter.hpp"],
    deps = [
        "//compiler",
        "//runtime:starlark_bigint",
        "//runtime:starlark_bool",
        "//runtime:starlark_bytes",
        "//runtime:starlark_dictionary",
        "//runtime:starlark_float",
        "//runtime:starlark_function",
        "//runtime:starlark_integer",
        "//runtime:starlark_list",
        "//runtime:starlark_none",
        "//runtime:starlark_numeric_impl",
        "//runtime:starlark_object",
        "//runtime:starlark_string",
        "//runtime:starlark_tuple",
        "//runtime:starlark_types",
    ],
)

cc_test(
    name = "interpreter_test",
    srcs = ["interpreter_test.cpp"],
    deps = [
        ":interpreter",
        "//runtime:starlark_bigint",
        "//runtime:starlark_bool",
        "//runtime:starlark_bytes",
        "//runtime:starlark_dictionary",
        "//runtime:starlark_float",
        "//runtime:starlark_integer",
        "//runtime:starlark_list",
        "//runtime:starlark_none",
        "//runtime:starlark_string",
        "//runtime:starlark_testing",
        "//runtime:starlark_tuple",
        "@googletest//:gtest_main",
    ],
)
