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
        ":utf8_reader",
        "@googletest//:gtest_main",
    ],
    size = "small",
)

cc_library(
    name = "utf8_reader",
    srcs = ["utf8_reader.cpp"],
    hdrs = ["utf8_reader.hpp"],
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

