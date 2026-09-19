# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "bytecode_golden_updater",
    testonly = True,
    srcs = ["bytecode_golden_updater.cpp"],
    hdrs = ["bytecode_golden_updater.hpp"],
    deps = [
        "//compiler",
        "//grammar:options",
        "//io:read_file",
        "//logging",
        "//proto/bytecode_formatter:bytecode_txtpb_printer",
        "//proto:starlark_bytecode_cc_proto",
        "@protobuf",
    ],
)

cc_binary(
    name = "bytecode_txtpb_tool",
    testonly = True,
    srcs = ["bytecode_txtpb_tool.cpp"],
    deps = [
        ":bytecode_golden_updater",
        "//proto/bytecode_formatter:bytecode_txtpb_printer",
        "//proto:starlark_bytecode_cc_proto",
        "@protobuf",
    ],
)
