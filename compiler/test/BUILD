# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "stack_depth_tests",
    srcs = glob(["stack_depth_tests/*.star"]),
)

cc_library(
    name = "stack_depth_test_support",
    testonly = True,
    hdrs = ["stack_depth_test_support.hpp"],
    deps = [
        "//compiler",
        "//compiler/analysis:stack_depth_analysis",
        "//grammar:options",
        "//logging",
        "//proto:starlark_bytecode_cc_proto",
    ],
)

cc_binary(
    name = "stack_depth_opcode_coverage_report",
    testonly = True,
    srcs = ["stack_depth_opcode_coverage_report.cpp"],
    data = [":stack_depth_tests"],
    deps = [":stack_depth_test_support"],
)

cc_test(
    name = "stack_depth_analysis_test",
    srcs = [
        "call_site_analysis_test.cpp",
        "stack_depth_opcode_effects_test.cpp",
    ],
    data = [":stack_depth_tests"],
    deps = [
        ":stack_depth_test_support",
        "//compiler/analysis:call_site_analysis",
        "//compiler/analysis:stack_depth_analysis",
        "@googletest//:gtest_main",
    ],
)
