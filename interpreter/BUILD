# Copyright 2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load("@rules_fuzzing//fuzzing:cc_defs.bzl", "cc_fuzz_test")

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
    visibility = ["//visibility:public"],
    deps = [
        ":built_in_functions",
        ":frame",
        "//proto:starlark_bytecode_cc_proto",
        "//runtime:starlark_function",
        "//runtime:starlark_object",
    ],
)

cc_library(
    name = "runner_state",
    hdrs = ["runner_state.hpp"],
    deps = [
        ":frame",
        ":module_loader",
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
    name = "built_in_functions",
    srcs = ["built_in_functions.cpp"],
    hdrs = ["built_in_functions.hpp"],
    deps = [
        "//runtime:starlark_function",
        "//runtime:starlark_object",
    ],
)

cc_library(
    name = "interpreter",
    srcs = ["interpreter.cpp"],
    hdrs = ["interpreter.hpp"],
    visibility = ["//visibility:public"],
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

cc_library(
    name = "interpreter_test_runner",
    srcs = ["interpreter_test_runner.cpp"],
    hdrs = ["interpreter_test_runner.hpp"],
    testonly = True,
    visibility = ["//visibility:public"],
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
    ],
)

cc_library(
    name = "interpreter_test_base",
    srcs = ["interpreter_test.cpp"],
    alwayslink = 1,
    testonly = True,
    deps = [
        ":interpreter_test_runner",
        "//third-party:defer",
        "@googletest//:gtest",
    ],
)

[
    cc_test(
        name = "interpreter_test_" + f.replace("/", "_"),
        args = [
            "$(location " + f + ")",
        ],
        data = [
            f,
        ],
        deps = [
            ":interpreter_test_base",
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
        "//third-party:defer",
    ],
)

cc_fuzz_test(
    name = "interpreter_fuzz_test",
    srcs = ["interpreter_fuzz_test.cpp"],
    corpus = [
        "//errors/test:corpus_filegroup",
        "//grammar/lexer_fuzz_test_corpus:corpus_filegroup",
        "//spec/tests:corpus_filegroup",
    ],
    dicts = [
        "//grammar/lexer_fuzz_test_dictionary:lexical_elements.dict",
    ],
    deps = [
        ":interpreter",
    ],
)

