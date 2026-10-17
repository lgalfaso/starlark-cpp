# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load("@rules_fuzzing//fuzzing:cc_defs.bzl", "cc_fuzz_test")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "engine",
    srcs = ["engine.cpp"],
    hdrs = ["engine.hpp"],
    deps = [
        "//interpreter",
        "//logging",
        "//native/runner:native_runner",
        "//native/runner:native_runtime",
        "//runtime:options",
        "//runtime:starlark_types",
        "//status_or:status",
        "//vm:module_loader",
        "//vm:split_test_case",
    ],
)

cc_library(
    name = "spec_support",
    srcs = ["spec_support.cpp"],
    hdrs = ["spec_support.hpp"],
    deps = [
        ":engine",
        "//runtime:options",
        "//vm:test_assertions",
        "@protobuf",
    ],
)

cc_test(
    name = "engine_test",
    size = "large",
    srcs = ["engine_test.cpp"],
    deps = [
        ":engine",
        "//runtime:options",
        "//vm:module_loader",
        "@googletest//:gtest_main",
    ],
)

cc_binary(
    name = "differential_check",
    testonly = True,
    srcs = ["check_main.cpp"],
    deps = [
        ":spec_support",
        "//io:read_file",
        "//runtime:options",
    ],
)

cc_fuzz_test(
    name = "fuzz_test",
    srcs = ["fuzz_test.cpp"],
    corpus = [
        "//errors/test:corpus_filegroup",
        "//grammar/lexer_fuzz_test_corpus:corpus_filegroup",
        "//grammar/parser_fuzz_test_corpus:corpus_filegroup",
        "//spec/tests:corpus_filegroup",
    ],
    dicts = [
        "//grammar/lexer_fuzz_test_dictionary:lexical_elements.dict",
    ],
    deps = [
        ":spec_support",
        "//runtime:options",
    ],
)
