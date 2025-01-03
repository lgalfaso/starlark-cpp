# Copyright 2024 Lucas Mirelmann

load("//unicode/extract:extract.bzl", "extract_ucd")
load("@rules_fuzzing//fuzzing:cc_defs.bzl", "cc_fuzz_test")

cc_library(
    name = "encode",
    srcs = ["encode.cpp"],
    hdrs = ["encode.hpp"],
    deps = [
        ":ucd",
        ":utf8_reader",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "encode_test",
    srcs = ["encode_test.cpp"],
    deps = [
        ":encode",
        ":ucd",
        ":utf8_reader",
        "@googletest//:gtest_main",
    ],
    size = "small",
)

cc_library(
    name = "normalization",
    srcs = ["normalization.cpp"],
    hdrs = ["normalization.hpp"],
    deps = [
        ":encode",
        ":ucd",
        ":utf8_reader",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "normalization_test",
    srcs = ["normalization_test.cpp"],
    deps = [
        ":normalization",
        ":ucd",
        "@googletest//:gtest_main",
    ],
    args = [
        "$(location @ucd//:NormalizationTest.txt)",
    ],
    data = [
        "@ucd//:NormalizationTest.txt",
    ],
    size = "small",
)

cc_library(
    name = "utf8_reader",
    srcs = ["utf8_reader.cpp"],
    hdrs = ["utf8_reader.hpp"],
    deps = [
        ":ucd",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "utf8_reader_test",
    srcs = ["utf8_reader_test.cpp"],
    deps = [
        ":utf8_reader",
        "@googletest//:gtest_main",
    ],
    size = "small",
)

cc_fuzz_test(
    name = "utf8_reader_fuzz_test",
    srcs = ["utf8_reader_fuzz_test.cpp"],
    deps = [
        ":utf8_reader",
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
    srcs = ["ucd_test.cpp"],
    deps = [
        ":ucd",
        "@googletest//:gtest_main",
    ],
    args = [
        "$(location @ucd//:DerivedCoreProperties.txt)",
    ],
    data = [
        "@ucd//:DerivedCoreProperties.txt",
    ],
    size = "small",
)

cc_binary(
    name = "ucd_benchmark",
    srcs = ["ucd_benchmark.cpp"],
    deps = [
        ":ucd",
        "@google_benchmark//:benchmark",
    ],
    testonly = True,
)

