# Copyright 2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

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

[
    cc_test(
        name = "compiler_bytecode_test_" + f.replace("/", "_"),
        size = "small",
        srcs = ["compiler_bytecode_test.cpp"],
        args = [
            "$(location " + f + ")",
            "$(location " + f.replace(".star", ".txtpb") + ")",
        ],
        data = [
            f,
            f.replace(".star", ".txtpb"),
        ],
        deps = [
            ":compiler",
            "//proto:starlark_bytecode_cc_proto",
            "@googletest//:gtest_main",
            "@protobuf",
        ],
    )
    for f in glob(["bytecode_generator_tests/*.star"])
]
