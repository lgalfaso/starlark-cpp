# Copyright 2024-2025 Lucas Mirelmann

package(
    default_visibility = ["//visibility:public"]
)

cc_library(
    name = "number",
    srcs = ["number.cpp"],
    hdrs = ["number.hpp"],
)

cc_test(
    name = "number_test",
    srcs = ["number_test.cpp"],
    deps = [
        ":number",
         "@googletest//:gtest_main",
    ],
    size = "large",
)

cc_binary(
    name = "number_benchmark",
    srcs = ["number_benchmark.cpp"],
    deps = [
        ":number",
        "@google_benchmark//:benchmark",
    ],
    testonly = True,
)
