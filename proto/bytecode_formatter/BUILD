# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "bytecode_txtpb_printer",
    srcs = ["bytecode_txtpb_printer.cpp"],
    hdrs = ["bytecode_txtpb_printer.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//proto:starlark_bytecode_cc_proto",
        "@protobuf",
    ],
)

cc_library(
    name = "bytecode_golden_updater",
    srcs = ["bytecode_golden_updater.cpp"],
    hdrs = ["bytecode_golden_updater.hpp"],
    testonly = True,
    visibility = ["//visibility:public"],
    deps = [
        ":bytecode_txtpb_printer",
        "//compiler",
        "//grammar:options",
        "//logging",
        "//proto:starlark_bytecode_cc_proto",
        "//third-party:defer",
        "@protobuf",
    ],
)

cc_binary(
    name = "bytecode_txtpb_formatter",
    testonly = True,
    srcs = ["bytecode_txtpb_formatter.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":bytecode_golden_updater",
        ":bytecode_txtpb_printer",
        "//proto:starlark_bytecode_cc_proto",
        "//third-party:defer",
        "@protobuf",
    ],
)
