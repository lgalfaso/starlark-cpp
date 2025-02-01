# Copyright 2024-2025 Lucas Mirelmann

load("@rules_fuzzing//fuzzing:cc_defs.bzl", "cc_fuzz_test")
load("//grammar:starlark_proto_encoder.bzl", "starlark_proto_encoder")

cc_library(
    name = "lexer",
    srcs = ["lexer.cpp"],
    hdrs = ["lexer.hpp"],
    deps = [
        ":logging",
        ":numeric_parser",
        ":options",
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
    dicts = [
        "lexer_fuzz_test_dictionary/lexical_elements.dict",
    ],
)

cc_library(
    name = "logging",
    srcs = ["logging.cpp"],
    hdrs = ["logging.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":token",
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
    name = "parser",
    srcs = ["parser.cpp"],
    hdrs = ["parser.hpp"],
    deps = [
        ":lexer",
        ":logging",
        ":options",
        "//grammar/proto:starlark_cc_proto",
        "//third-party:defer",
        "//unicode:normalization",
    ],
)

cc_binary(
    name = "parser_proto_encoder",
    srcs = ["parser_proto_encoder.cpp"],
    deps = [
        "@protobuf//:protobuf",
        "//grammar/proto:starlark_cc_proto",
        "//third-party:defer",
    ],
    testonly = True,
)

cc_binary(
    name = "parser_main",
    srcs = ["parser_main.cpp"],
    deps = [
        ":parser",
        "@protobuf//:protobuf",
        "//grammar/proto:starlark_cc_proto",
        "//third-party:defer",
    ],
)

[[
  starlark_proto_encoder(
      name = "parser_test_encoder_" + f.replace("/", "_"),
      input = [f.removesuffix(".star") + ".txtpb"],
      testonly = True,
  ),
  cc_test(
      name = "parser_test_" + f.replace("/", "_"),
      srcs = ["parser_test_check.cpp"],
      args = [
          "$(location " + f + ")",
          "$(location :parser_test_encoder_" + f.replace("/", "_") + ")",
      ],
      deps = [
          ":parser",
          ":parsing_options",
          "@googletest//:gtest_main",
          "@protobuf//:protobuf",
          "//grammar/proto:starlark_cc_proto",
          "//third-party:defer",
      ],
      data = [
          f,
          ":parser_test_encoder_" + f.replace("/", "_"),
      ],
  )] for f in glob(["parser_tests/*.star"])]

[
  cc_test(
      name = "parser_test_" + f.replace("/", "_"),
      srcs = ["parser_test_valid.cpp"],
      args = [
          "$(location " + f + ")",
      ],
      deps = [
          ":parser",
          ":parsing_options",
          "@googletest//:gtest_main",
          "@protobuf//:protobuf",
          "//grammar/proto:starlark_cc_proto",
          "//third-party:defer",
      ],
      data = [
          f,
      ],
  ) for f in glob(["parser_tests_valid/*.star"])
]

[
  cc_test(
      name = "parser_test_" + f.replace("/", "_"),
      srcs = ["parser_test_invalid.cpp"],
      args = [
          "$(location " + f + ")",
      ],
      deps = [
          ":parser",
          ":parsing_options",
          "@googletest//:gtest_main",
          "@protobuf//:protobuf",
          "//grammar/proto:starlark_cc_proto",
          "//third-party:defer",
      ],
      data = [
          f,
      ],
  ) for f in glob(["parser_tests_invalid/*.star"])
]

cc_test(
    name = "parser_test",
    srcs = ["parser_test.cpp"],
    deps = [
        ":parser",
        "@googletest//:gtest_main",
        "@protobuf//:protobuf",
        "//grammar/proto:starlark_cc_proto",
    ],
)

cc_fuzz_test(
    name = "parser_fuzz_test",
    srcs = ["parser_fuzz_test.cpp"],
    deps = [
        ":parser",
    ],
    corpus = [
        "//grammar/parser_fuzz_test_corpus:corpus_filegroup",
    ],
    dicts = [
        "lexer_fuzz_test_dictionary/lexical_elements.dict",
    ],
)

cc_library(
    name = "options",
    hdrs = ["options.hpp"],
)

cc_library(
    name = "parsing_options",
    srcs = ["parsing_options.cpp"],
    hdrs = ["parsing_options.hpp"],
    deps = [
        ":options",
    ],
    testonly = True,
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
    deps = [
        "//bignum:number",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "token_test",
    srcs = ["token_test.cpp"],
    deps = [
        ":token",
        "@googletest//:gtest_main",
    ],
    size = "small",
)

