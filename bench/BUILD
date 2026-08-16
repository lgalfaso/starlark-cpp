# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "benchmark_interpreter",
    testonly = True,
    srcs = ["benchmark_main.cpp"],
    data = glob(["*.star"]),
    deps = [
        "//interpreter",
        "//third-party:defer",
        "@google_benchmark//:benchmark",
    ],
)

