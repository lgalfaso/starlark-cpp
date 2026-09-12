# Copyright 2024-2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load("@rules_fuzzing//fuzzing:cc_defs.bzl", "cc_fuzz_test")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "number",
    srcs = ["number.cpp"],
    hdrs = ["number.hpp"],
)

cc_library(
    name = "number_stream_for_test",
    testonly = True,
    srcs = ["number_stream_for_test.cpp"],
    hdrs = ["number_stream_for_test.hpp"],
    deps = [
        ":number",
    ],
)

cc_test(
    name = "number_test",
    size = "large",
    srcs = ["number_test.cpp"],
    shard_count = 48,
    deps = [
        ":number",
        ":number_stream_for_test",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "number_logical_or_test",
    size = "large",
    srcs = ["number_logical_or_test.cpp"],
    deps = [
        ":number",
        ":number_stream_for_test",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "number_logical_and_test",
    size = "large",
    srcs = ["number_logical_and_test.cpp"],
    deps = [
        ":number",
        ":number_stream_for_test",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "number_logical_xor_test",
    size = "large",
    srcs = ["number_logical_xor_test.cpp"],
    deps = [
        ":number",
        ":number_stream_for_test",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "number_logical_not_test",
    size = "large",
    srcs = ["number_logical_not_test.cpp"],
    deps = [
        ":number",
        ":number_stream_for_test",
        "@googletest//:gtest_main",
    ],
)

cc_binary(
    name = "number_benchmark",
    testonly = True,
    srcs = ["number_benchmark.cpp"],
    deps = [
        ":number",
        "@google_benchmark//:benchmark",
    ],
)

cc_binary(
    name = "number_karatsuba_benchmark",
    testonly = True,
    srcs = ["number_karatsuba_benchmark.cpp"],
    deps = [
        ":number",
        "@google_benchmark//:benchmark",
    ],
)

cc_fuzz_test(
    name = "number_div_fuzz_test",
    srcs = ["number_div_fuzz_test.cpp"],
    dicts = [
        "dictionaries/numbers.dict",
    ],
    deps = [
        ":number",
    ],
)
