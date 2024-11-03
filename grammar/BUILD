# Copyright 2024 Lucas Mirelmann

load("@rules_fuzzing//fuzzing:cc_defs.bzl", "cc_fuzz_test")

cc_library(
    name = "lexer",
    srcs = ["lexer.cpp"],
    hdrs = ["lexer.hpp"],
    deps = [
        ":numeric_parser",
        ":token",
        "//unicode:encode",
        "//unicode:ucd",
        "//unicode:utf8_reader",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "lexer_test",
    srcs = ["lexer_test.cpp"],
    deps = [
        ":lexer",
        ":quoted",
        "@googletest//:gtest_main",
    ],
    size = "small",
)

cc_fuzz_test(
    name = "lexer_fuzz_test",
    srcs = ["lexer_fuzz_test.cpp"],
    deps = [
        ":lexer",
    ],
    corpus = [
        "//grammar/lexer_fuzz_test_corpus:corpus_filegroup",
    ],
)

cc_library(
    name = "numeric_parser",
    srcs = ["numeric_parser.cpp"],
    hdrs = ["numeric_parser.hpp"],
    deps = [
        "//unicode:utf8_reader",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "numeric_parser_test",
    srcs = ["numeric_parser_test.cpp"],
    deps = [
        ":numeric_parser",
        "@googletest//:gtest_main",
    ],
    size = "small",
)

cc_library(
    name = "quoted",
    srcs = ["quoted.cpp"],
    hdrs = ["quoted.hpp"],
)

cc_test(
    name = "quoted_test",
    srcs = ["quoted_test.cpp"],
    deps = [
        ":quoted",
        "@googletest//:gtest_main",
    ],
    size = "small",
)

cc_library(
    name = "token",
    srcs = ["token.cpp"],
    hdrs = ["token.hpp"],
    visibility = ["//visibility:public"],
)

