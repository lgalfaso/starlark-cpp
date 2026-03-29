# Copyright 2024-2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load("@rules_fuzzing//fuzzing:cc_defs.bzl", "cc_fuzz_test")
load("//proto:starlark_proto_encoder.bzl", "starlark_ast_proto_encoder")

cc_library(
    name = "ast_listener",
    srcs = ["ast_listener.cpp"],
    hdrs = ["ast_listener.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//proto:starlark_ast_cc_proto",
    ],
)

[
    cc_test(
        name = "ast_listener_test_" + f.replace("/", "_"),
        size = "small",
        srcs = ["ast_listener_test.cpp"],
        args = [
            "$(location " + f + ")",
            "$(location " + f.replace(".star", ".txt") + ")",
        ],
        data = [
            f,
            f.replace(".star", ".txt"),
        ],
        deps = [
            ":ast_listener",
            ":parser",
            ":parsing_options",
            ":quoted",
            "//proto:starlark_ast_cc_proto",
            "//third-party:defer",
            "@googletest//:gtest_main",
            "@protobuf",
        ],
    )
    for f in glob(["ast_listener_tests/*.star"])
]

cc_library(
    name = "lexer",
    srcs = ["lexer.cpp"],
    hdrs = ["lexer.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":numeric_parser",
        ":options",
        ":token",
        "//logging",
        "//unicode:encode",
        "//unicode:ucd",
        "//unicode:utf8_reader",
    ],
)

cc_test(
    name = "lexer_test",
    size = "small",
    srcs = ["lexer_test.cpp"],
    deps = [
        ":lexer",
        ":quoted",
        "@googletest//:gtest_main",
    ],
)

cc_fuzz_test(
    name = "lexer_fuzz_test",
    srcs = ["lexer_fuzz_test.cpp"],
    corpus = [
        "//grammar/lexer_fuzz_test_corpus:corpus_filegroup",
    ],
    dicts = [
        "lexer_fuzz_test_dictionary/lexical_elements.dict",
    ],
    deps = [
        ":lexer",
    ],
)

cc_library(
    name = "numeric_parser",
    srcs = ["numeric_parser.cpp"],
    hdrs = ["numeric_parser.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//status_or:status",
        "//unicode:utf8_reader",
    ],
)

cc_test(
    name = "numeric_parser_test",
    size = "small",
    srcs = ["numeric_parser_test.cpp"],
    deps = [
        ":numeric_parser",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.cpp"],
    hdrs = ["parser.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":lexer",
        ":options",
        "//logging",
        "//proto:starlark_ast_cc_proto",
        "//third-party:defer",
        "//unicode:normalization",
    ],
)

cc_binary(
    name = "parser_main",
    srcs = ["parser_main.cpp"],
    deps = [
        ":parser",
        "//proto:starlark_ast_cc_proto",
        "//third-party:defer",
        "@protobuf",
    ],
)

[[
    starlark_ast_proto_encoder(
        name = "parser_test_encoder_" + f.replace("/", "_"),
        testonly = True,
        input = [f.removesuffix(".star") + ".txtpb"],
    ),
    cc_test(
        name = "parser_test_" + f.replace("/", "_"),
        size = "small",
        srcs = ["parser_test_check.cpp"],
        args = [
            "$(location " + f + ")",
            "$(location :parser_test_encoder_" + f.replace("/", "_") + ")",
        ],
        data = [
            f,
            ":parser_test_encoder_" + f.replace("/", "_"),
        ],
        deps = [
            ":parser",
            ":parsing_options",
            "//proto:starlark_ast_cc_proto",
            "//third-party:defer",
            "@googletest//:gtest_main",
            "@protobuf",
            "@protobuf-matchers//protobuf-matchers",
        ],
    ),
] for f in glob(["parser_tests/*.star"])]

[
    cc_test(
        name = "parser_test_" + f.replace("/", "_"),
        size = "small",
        srcs = ["parser_test_valid.cpp"],
        args = [
            "$(location " + f + ")",
        ],
        data = [
            f,
        ],
        deps = [
            ":parser",
            ":parsing_options",
            "//proto:starlark_ast_cc_proto",
            "//third-party:defer",
            "@googletest//:gtest_main",
            "@protobuf",
        ],
    )
    for f in glob(["parser_tests_valid/*.star"])
]

[
    cc_test(
        name = "parser_test_" + f.replace("/", "_"),
        size = "small",
        srcs = ["parser_test_invalid.cpp"],
        args = [
            "$(location " + f + ")",
        ],
        data = [
            f,
        ],
        deps = [
            ":parser",
            ":parsing_options",
            "//proto:starlark_ast_cc_proto",
            "//third-party:defer",
            "@googletest//:gtest_main",
            "@protobuf",
        ],
    )
    for f in glob(["parser_tests_invalid/*.star"])
]

cc_test(
    name = "parser_test",
    size = "small",
    srcs = ["parser_test.cpp"],
    deps = [
        ":parser",
        "//proto:starlark_ast_cc_proto",
        "@googletest//:gtest_main",
        "@protobuf",
    ],
)

cc_fuzz_test(
    name = "parser_fuzz_test",
    srcs = ["parser_fuzz_test.cpp"],
    corpus = [
        "//grammar/parser_fuzz_test_corpus:corpus_filegroup",
    ],
    dicts = [
        "lexer_fuzz_test_dictionary/lexical_elements.dict",
    ],
    deps = [
        ":parser",
    ],
)

cc_library(
    name = "options",
    srcs = ["options.cpp"],
    hdrs = ["options.hpp"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "parsing_options",
    testonly = True,
    srcs = ["parsing_options.cpp"],
    hdrs = ["parsing_options.hpp"],
    deps = [
        ":options",
    ],
)

cc_library(
    name = "quoted",
    srcs = ["quoted.cpp"],
    hdrs = ["quoted.hpp"],
)

cc_test(
    name = "quoted_test",
    size = "small",
    srcs = ["quoted_test.cpp"],
    deps = [
        ":quoted",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "token",
    srcs = ["token.cpp"],
    hdrs = ["token.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//bigint:number",
        "//proto:starlark_logging_cc_proto",
    ],
)

cc_test(
    name = "token_test",
    size = "small",
    srcs = ["token_test.cpp"],
    deps = [
        ":token",
        "@googletest//:gtest_main",
    ],
)
