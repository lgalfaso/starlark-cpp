# Copyright 2025 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "bytecode",
    srcs = ["bytecode.cpp"],
    hdrs = ["bytecode.hpp"],
    deps = [
        "//bigint:number",
    ],
)

cc_library(
    name = "compiler",
    srcs = ["compiler.cpp"],
    hdrs = ["compiler.hpp"],
    deps = [
        ":bytecode",
        "//grammar:ast_listener",
        "//grammar:logging",
        "//grammar:options",
        "//grammar:parser",
        "//proto:starlark_ast_cc_proto",
    ],
)
