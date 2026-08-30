# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "read_file",
    srcs = ["read_file.cpp"],
    hdrs = ["read_file.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//third-party:defer",
    ],
)

cc_test(
    name = "read_file_test",
    size = "small",
    srcs = ["read_file_test.cpp"],
    deps = [
        ":read_file",
        "@googletest//:gtest_main",
    ],
)
