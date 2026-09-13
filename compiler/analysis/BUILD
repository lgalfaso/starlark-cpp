# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library")

filegroup(
    name = "stack_depth_tests",
    srcs = glob(["stack_depth_tests/*.star"]),
    visibility = ["//visibility:public"],
)

cc_library(
    name = "stack_depth_analysis",
    srcs = ["stack_depth_analysis.cpp"],
    hdrs = ["stack_depth_analysis.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//proto:starlark_bytecode_cc_proto",
    ],
)

cc_library(
    name = "call_site_analysis",
    srcs = ["call_site_analysis.cpp"],
    hdrs = ["call_site_analysis.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":stack_depth_analysis",
        "//proto:starlark_bytecode_cc_proto",
    ],
)
