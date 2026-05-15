# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

[
    cc_test(
        name = "spec_interpreter_test_" + f.replace("/", "_"),
        srcs = ["spec_interpreter_test.cpp"],
        args = [
            "$(location " + f + ")",
        ],
        data = [
            f,
        ],
        deps = [
            "//interpreter:interpreter_test_runner",
            "@googletest//:gtest_main",
        ],
        tags = ["spec"],
    ) for f in glob(["tests/*.star"])
]

