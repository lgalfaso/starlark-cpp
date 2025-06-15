# Copyright 2024-2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "extract",
    srcs = ["extract.cpp"],
    hdrs = ["extract.hpp"],
)

cc_binary(
    name = "extract_main",
    srcs = ["extract_main.cpp"],
    deps = [
        ":extract",
    ],
)
