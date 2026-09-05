# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

cc_library(
    name = "spec_interpreter_test_base",
    srcs = ["spec_interpreter_test.cpp"],
    visibility = ["//visibility:public"],
    alwayslink = 1,
    testonly = True,
    deps = [
        "//interpreter:interpreter_test_runner",
        "//io:read_file",
        "@googletest//:gtest",
    ],
)

cc_binary(
    name = "spec_interpreter_test",
    testonly = True,
    visibility = ["//spec:__subpackages__"],
    deps = [
        ":spec_interpreter_test_base",
        "@googletest//:gtest_main",
    ],
)

