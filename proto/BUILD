# Copyright 2024-2025 Lucas Mirelmann

load("@protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")
load("@protobuf//bazel:proto_library.bzl", "proto_library")

proto_library(
    name = "starlark_ast_proto",
    srcs = ["starlark_ast.proto"],
    visibility = ["//visibility:public"],
    deps = [
        "@protobuf//:any_proto",
    ],
)

cc_proto_library(
    name = "starlark_ast_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":starlark_ast_proto"],
)
