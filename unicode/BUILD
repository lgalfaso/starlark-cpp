# Copyright 2024 Lucas Mirelmann

load("//unicode/extract:extract.bzl", "extract_ucd")

cc_library(
    name = "encode",
    srcs = ["encode.cpp"],
    hdrs = ["encode.hpp"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "encode_test",
    srcs = ["encode_test.cpp"],
    deps = [
        ":encode",
        "@googletest//:gtest_main",
        "//grammar:source",
    ],
    size = "small",
)

extract_ucd(
    name = "ucd_extract",
    output_cc = "ucd_codepoints.cpp",
    output_h = "ucd_codepoints.hpp",
)

cc_library(
    name = "ucd",
    srcs = ["ucd_codepoints.cpp"],
    hdrs = ["ucd_codepoints.hpp"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "ucd_test",
    srcs = ["ucd_test.cpp"],
    deps = [
        ":ucd",
        "@googletest//:gtest_main",
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

