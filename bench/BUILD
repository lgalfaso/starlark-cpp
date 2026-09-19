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
        "//native/runner:native_options",
        "//native/runner:native_runner",
        "//native/runner:native_runtime",
        "//vm:module_loader",
        "@google_benchmark//:benchmark",
    ],
)
