# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "benchmark",
    testonly = True,
    srcs = ["benchmark_main.cpp"],
    data = glob(["*.star"]),
    deps = [
        "//grammar:options",
        "//interpreter",
        "//io:read_file",
        "//vm:module_loader",
        "@google_benchmark//:benchmark",
    ],
)
