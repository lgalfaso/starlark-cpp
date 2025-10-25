# Copyright 2024-2025 Lucas Mirelmann

load("@protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")
load("@protobuf//bazel:proto_library.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

proto_library(
    name = "starlark_ast_proto",
    srcs = ["starlark_ast.proto"],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "starlark_ast_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":starlark_ast_proto"],
)

proto_library(
    name = "starlark_bytecode_proto",
    srcs = ["starlark_bytecode.proto"],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "starlark_bytecode_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":starlark_bytecode_proto"],
)

proto_library(
    name = "starlark_logging_proto",
    srcs = ["starlark_logging.proto"],
    visibility = ["//visibility:public"],
    deps = ["@protobuf//:timestamp_proto"],
)

cc_proto_library(
    name = "starlark_logging_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":starlark_logging_proto"],
)

cc_library(
    name = "base_proto_encoder",
    testonly = True,
    hdrs = ["base_proto_encoder.hpp"],
    deps = [
        "//third-party:defer",
        "@protobuf",
    ],
)

cc_binary(
    name = "ast_proto_encoder",
    testonly = True,
    srcs = ["ast_proto_encoder.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":base_proto_encoder",
        "//proto:starlark_ast_cc_proto",
    ],
)

cc_binary(
    name = "bytecode_proto_encoder",
    testonly = True,
    srcs = ["bytecode_proto_encoder.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":base_proto_encoder",
        "//proto:starlark_bytecode_cc_proto",
    ],
)
