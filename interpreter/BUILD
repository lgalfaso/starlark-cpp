# Copyright 2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

cc_library(
    name = "frame",
    srcs = ["frame.cpp"],
    hdrs = ["frame.hpp"],
    deps = [
        "//runtime:starlark_object",
        "@protobuf",
    ],
)

cc_library(
    name = "module_loader",
    srcs = ["module_loader.cpp"],
    hdrs = [
        "module_loader.hpp",
        "builtin.star",
    ],
    deps = [
        ":frame",
        "//proto:starlark_bytecode_cc_proto",
        "//runtime:starlark_object",
    ],
)

cc_library(
    name = "runner_state",
    hdrs = ["runner_state.hpp"],
    deps = [
        ":frame",
        "//proto:starlark_bytecode_cc_proto",
        "//runtime:starlark_object",
    ],
)

cc_library(
    name = "function",
    srcs = ["function.cpp"],
    hdrs = ["function.hpp"],
    deps = [
        ":frame",
        ":runner_state",
        "//proto:starlark_bytecode_cc_proto",
        "//runtime:starlark_dictionary",
        "//runtime:starlark_function",
        "//runtime:starlark_object",
        "//runtime:starlark_string",
        "//runtime:starlark_tuple",
    ],
)

cc_library(
    name = "interpreter",
    srcs = ["interpreter.cpp"],
    hdrs = ["interpreter.hpp"],
    deps = [
        ":frame",
        ":function",
        ":module_loader",
        ":runner_state",
        "//compiler",
        "//runtime:starlark_bigint",
        "//runtime:starlark_bool",
        "//runtime:starlark_bytes",
        "//runtime:starlark_dictionary",
        "//runtime:starlark_float",
        "//runtime:starlark_function",
        "//runtime:starlark_function_builder_impl",
        "//runtime:starlark_integer",
        "//runtime:starlark_list",
        "//runtime:starlark_none",
        "//runtime:starlark_numeric_impl",
        "//runtime:starlark_object",
        "//runtime:starlark_object_impl",
        "//runtime:starlark_string",
        "//runtime:starlark_tuple",
        "//runtime:starlark_types",
        "@bazel_tools//tools/cpp/runfiles",
    ],
)

[
    cc_test(
        name = "interpreter_test_" + f.replace("/", "_"),
        srcs = ["interpreter_test.cpp"],
        args = [
            "$(location " + f + ")",
        ],
        data = [
            f,
        ],
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
    ) for f in glob(["interpreter_tests/*.star"])
]

cc_test(
    name = "module_test",
    srcs = ["module_test.cpp"],
    deps = [
        ":interpreter",
        "@googletest//:gtest_main",
    ],
)

cc_binary(
    name = "interpreter_main",
    srcs = ["interpreter_main.cpp"],
    deps = [
        ":interpreter",
    ],
)

