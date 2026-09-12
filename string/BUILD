# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "levenshtein",
    srcs = ["levenshtein.cpp"],
    hdrs = ["levenshtein.hpp"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "levenshtein_test",
    size = "small",
    srcs = ["levenshtein_test.cpp"],
    deps = [
        ":levenshtein",
        "@googletest//:gtest_main",
    ],
)
