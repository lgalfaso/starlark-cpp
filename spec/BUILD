# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "spec_interpreter_test_base",
    srcs = ["spec_interpreter_test.cpp"],
    visibility = ["//visibility:public"],
    alwayslink = 1,
    testonly = True,
    deps = [
        "//interpreter:interpreter_test_runner",
        "@googletest//:gtest_main",
    ],
)

