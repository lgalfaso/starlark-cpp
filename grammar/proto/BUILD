# Copyright 2024-2025 Lucas Mirelmann

proto_library(
    name = "starlark_proto",
    srcs = ["starlark.proto"],
    visibility = ["//visibility:public"],
    deps = [
        "@protobuf//:any_proto",
    ],
)

cc_proto_library(
    name = "starlark_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":starlark_proto"],
)
