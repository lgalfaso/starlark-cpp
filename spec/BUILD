# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "spec_interpreter_test_base",
    srcs = ["spec_interpreter_test.cpp"],
    alwayslink = 1,
    testonly = True,
    deps = [
        "//interpreter:interpreter_test_runner",
        "@googletest//:gtest_main",
    ],
)
[
    cc_test(
        name = "spec_interpreter_test_" + f.replace("/", "_"),
        args = [
            "$(location " + f + ")",
        ],
        data = [
            f,
        ],
        deps = [
            ":spec_interpreter_test_base",
            "@googletest//:gtest_main",
        ],
        tags = ["spec"],
    ) for f in glob(["tests/*.star"])
]

