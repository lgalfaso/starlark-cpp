# Copyright 2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("//proto:starlark_proto_encoder.bzl", "starlark_bytecode_proto_encoder")

cc_library(
    name = "compiler",
    srcs = ["compiler.cpp"],
    hdrs = ["compiler.hpp"],
    deps = [
        "//grammar:ast_listener",
        "//grammar:logging",
        "//grammar:options",
        "//grammar:parser",
        "//proto:starlark_ast_cc_proto",
        "//proto:starlark_bytecode_cc_proto",
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
        srcs = ["compiler_bytecode_test.cpp"],
        args = [
            "$(location " + f + ")",
            "$(location :compiler_bytecode_test_encoder_" + f.replace("/", "_") + ")",
        ],
        data = [
            f,
            ":compiler_bytecode_test_encoder_" + f.replace("/", "_"),
        ],
        deps = [
            ":compiler",
            "//proto:starlark_bytecode_cc_proto",
            "@googletest//:gtest_main",
            "@protobuf",
            "@protobuf-matchers//protobuf-matchers",
        ],
    )
] for f in glob(["bytecode_generator_tests/*.star"])]
