# Copyright 2024-2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "number",
    srcs = ["number.cpp"],
    hdrs = ["number.hpp"],
)

cc_test(
    name = "number_test",
    size = "large",
    srcs = ["number_test.cpp"],
    deps = [
        ":number",
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
