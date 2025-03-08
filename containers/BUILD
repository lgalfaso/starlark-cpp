# Copyright 2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "flat_map",
    hdrs = ["flat_map.hpp"],
)

cc_test(
    name = "flat_map_test",
    size = "small",
    srcs = ["flat_map_test.cpp"],
    deps = [
        ":flat_map",
        "@googletest//:gtest_main",
    ],
)

