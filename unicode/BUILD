# Copyright 2024-2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load("@rules_fuzzing//fuzzing:cc_defs.bzl", "cc_fuzz_test")
load("//unicode/extract:extract.bzl", "extract_ucd")

cc_library(
    name = "encode",
    srcs = ["encode.cpp"],
    hdrs = ["encode.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":ucd",
        ":utf8_reader",
    ],
)

cc_test(
    name = "encode_test",
    size = "small",
    srcs = ["encode_test.cpp"],
    deps = [
        ":encode",
        ":ucd",
        ":utf8_reader",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "normalization",
    srcs = ["normalization.cpp"],
    hdrs = ["normalization.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":encode",
        ":ucd",
        ":utf8_reader",
    ],
)

cc_test(
    name = "normalization_test",
    size = "small",
    srcs = ["normalization_test.cpp"],
    args = [
        "$(location @ucd//:NormalizationTest.txt)",
    ],
    data = [
        "@ucd//:NormalizationTest.txt",
    ],
    deps = [
        ":normalization",
        ":ucd",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "utf8_reader",
    srcs = ["utf8_reader.cpp"],
    hdrs = ["utf8_reader.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":ucd",
    ],
)

cc_test(
    name = "utf8_reader_test",
    size = "small",
    srcs = ["utf8_reader_test.cpp"],
    deps = [
        ":encode",
        ":utf8_reader",
        "@googletest//:gtest_main",
    ],
)

cc_fuzz_test(
    name = "utf8_reader_fuzz_test",
    size = "small",
    srcs = ["utf8_reader_fuzz_test.cpp"],
    corpus = [
        "//unicode/utf8_corpus:corpus_filegroup",
    ],
    deps = [
        ":utf8_reader",
    ],
)

cc_library(
    name = "utf8_reverse_reader",
    srcs = ["utf8_reverse_reader.cpp"],
    hdrs = ["utf8_reverse_reader.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":utf8_reader",
        ":ucd",
    ],
)

cc_test(
    name = "utf8_reverse_reader_test",
    size = "small",
    srcs = ["utf8_reverse_reader_test.cpp"],
    deps = [
        ":encode",
        ":utf8_reverse_reader",
        "@googletest//:gtest_main",
    ],
)

extract_ucd(
    name = "ucd_extract",
    output_cc = "ucd_code_points.cpp",
    output_h = "ucd_code_points.hpp",
)

cc_library(
    name = "ucd",
    srcs = ["ucd_code_points.cpp"],
    hdrs = ["ucd_code_points.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//containers:flat_map",
    ],
)

cc_test(
    name = "ucd_test",
    size = "small",
    srcs = ["ucd_test.cpp"],
    args = [
        "$(location @ucd//:DerivedCoreProperties.txt)",
        "$(location @ucd//:PropList.txt)",
        "$(location @ucd//:auxiliary/WordBreakProperty.txt)",
    ],
    data = [
        "@ucd//:DerivedCoreProperties.txt",
        "@ucd//:PropList.txt",
        "@ucd//:auxiliary/WordBreakProperty.txt",
    ],
    deps = [
        ":ucd",
        "//unicode/extract",
        "@googletest//:gtest_main",
    ],
)

cc_binary(
    name = "ucd_benchmark",
    testonly = True,
    srcs = ["ucd_benchmark.cpp"],
    deps = [
        ":ucd",
        "@google_benchmark//:benchmark",
    ],
)

cc_library(
    name = "word_break",
    srcs = ["word_break.cpp"],
    hdrs = ["word_break.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":ucd",
    ],
)

cc_test(
    name = "word_break_test",
    size = "small",
    srcs = ["word_break_test.cpp"],
    args = [
        "$(location @ucd//:auxiliary/WordBreakTest.txt)",
    ],
    data = [
        "@ucd//:auxiliary/WordBreakTest.txt",
    ],
    deps = [
        ":ucd",
        ":word_break",
        "//unicode/extract",
        "@googletest//:gtest_main",
    ],
)


