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

cc_library(
    name = "linked_hash_set",
    hdrs = ["linked_hash_set.hpp"],
)

cc_test(
    name = "linked_hash_set_test",
    size = "small",
    srcs = ["linked_hash_set_test.cpp"],
    deps = [
        ":linked_hash_set",
        "@googletest//:gtest_main",
    ],
)

