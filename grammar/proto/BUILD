# Copyright 2024-2025 Lucas Mirelmann

proto_library(
    name = "starlark_proto",
    srcs = ["starlark.proto"],
    deps = [
        "@protobuf//:any_proto",
    ],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "starlark_cc_proto",
    deps = [":starlark_proto"],
    visibility = ["//visibility:public"],
)

