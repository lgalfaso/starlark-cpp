# Copyright 2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load("//proto:starlark_proto_encoder.bzl", "starlark_bytecode_proto_encoder")

cc_library(
    name = "compiler",
    srcs = ["compiler.cpp"],
    hdrs = ["compiler.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//grammar:ast_listener",
        "//grammar:options",
        "//grammar:parser",
        "//logging",
        "//proto:starlark_ast_cc_proto",
        "//proto:starlark_bytecode_cc_proto",
    ],
)

cc_library(
    name = "compiler_bytecode_test_base",
    srcs = ["compiler_bytecode_test.cpp"],
    alwayslink = 1,
    testonly = True,
    deps = [
        ":compiler",
        "//proto:starlark_bytecode_cc_proto",
        "//third-party:defer",
        "@googletest//:gtest",
        "@protobuf",
        "@protobuf-matchers//protobuf-matchers",
    ],
)

[[
    starlark_bytecode_proto_encoder(
        name = "compiler_bytecode_test_encoder_" + f.replace("/", "_"),
        testonly = True,
        input = [f.removesuffix(".star") + ".txtpb"],
    ),
    cc_test(
        name = "compiler_bytecode_test_" + f.replace("/", "_"),
        size = "small",
        args = [
            "$(location " + f + ")",
            "$(location :compiler_bytecode_test_encoder_" + f.replace("/", "_") + ")",
        ],
        data = [
            f,
            ":compiler_bytecode_test_encoder_" + f.replace("/", "_"),
        ],
        deps = [
            ":compiler_bytecode_test_base",
            "@googletest//:gtest_main",
        ],
    ),
] for f in glob(["bytecode_generator_tests/*.star"])]

cc_binary(
    name = "update_bytecode_goldens",
    testonly = True,
    srcs = ["update_bytecode_goldens.cpp"],
    tags = ["manual"],
    deps = ["//proto/bytecode_formatter:bytecode_golden_updater"],
)
