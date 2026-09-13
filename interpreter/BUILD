# Copyright 2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load("@rules_fuzzing//fuzzing:cc_defs.bzl", "cc_fuzz_test")

cc_library(
    name = "runner_state",
    hdrs = ["runner_state.hpp"],
    deps = [
        "//proto:starlark_bytecode_cc_proto",
        "//runtime:starlark_object",
        "//vm:frame",
        "//vm:module_loader",
        "//vm:module_metadata",
    ],
)

cc_library(
    name = "function",
    srcs = ["function.cpp"],
    hdrs = ["function.hpp"],
    deps = [
        ":runner_state",
        "//proto:starlark_bytecode_cc_proto",
        "//runtime:starlark_dictionary",
        "//runtime:starlark_function",
        "//runtime:starlark_object",
        "//runtime:starlark_string",
        "//runtime:starlark_tuple",
        "//vm:frame",
        "//vm:function_call_binding",
        "//vm:module_metadata",
    ],
)

cc_library(
    name = "interpreter",
    srcs = ["interpreter.cpp"],
    hdrs = ["interpreter.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":function",
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
        "//vm:fast_compare",
        "//vm:frame",
        "//vm:module_load_tracker",
        "//vm:module_loader",
        "//vm:module_metadata",
        "//vm:predeclared_context",
        "//vm:program_limits",
        "@bazel_tools//tools/cpp/runfiles",
    ],
)

cc_library(
    name = "interpreter_test_runner",
    testonly = True,
    srcs = ["interpreter_test_runner.cpp"],
    hdrs = ["interpreter_test_runner.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":interpreter",
        "//grammar:options",
        "//runtime:starlark_function",
        "//runtime:starlark_types",
        "//vm:test_assertions",
        "//vm:test_case",
        "@protobuf",
    ],
)

cc_binary(
    name = "interpreter_test",
    testonly = True,
    srcs = ["interpreter_test.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":interpreter_test_runner",
        "//io:read_file",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

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
        "//io:read_file",
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
