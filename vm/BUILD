# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "module_metadata",
    srcs = ["module_metadata.cpp"],
    hdrs = ["module_metadata.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//proto:starlark_bytecode_cc_proto",
    ],
)

cc_library(
    name = "frame",
    srcs = ["frame.cpp"],
    hdrs = ["frame.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//runtime:starlark_object",
        "@protobuf",
    ],
)

cc_library(
    name = "built_in_functions",
    srcs = ["built_in_functions.cpp"],
    hdrs = ["built_in_functions.hpp"],
    deps = [
        "//errors:runtime_error_messages",
        "//runtime:starlark_function",
        "//runtime:starlark_list",
        "//runtime:starlark_object",
        "//runtime:starlark_types",
    ],
)

cc_library(
    name = "test_case",
    testonly = True,
    srcs = ["test_case.cpp"],
    hdrs = ["test_case.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":module_loader",
    ],
)

cc_library(
    name = "test_assertions",
    testonly = True,
    srcs = ["test_assertions.cpp"],
    hdrs = ["test_assertions.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":test_case",
        "//logging",
        "//runtime:options",
        "//runtime:starlark_function",
        "//runtime:starlark_string",
        "//runtime:starlark_types",
        "@protobuf",
    ],
)

cc_library(
    name = "program_limits",
    srcs = ["program_limits.cpp"],
    hdrs = ["program_limits.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//errors:runtime_error_messages",
        "//proto:starlark_bytecode_cc_proto",
        "//runtime:options",
    ],
)

cc_library(
    name = "module_loader",
    srcs = ["module_loader.cpp"],
    hdrs = [
        "builtin.star",
        "module_loader.hpp",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":built_in_functions",
        ":frame",
        ":module_metadata",
        "//proto:starlark_bytecode_cc_proto",
        "//runtime:starlark_function",
        "//runtime:starlark_object",
        "//status_or:status",
    ],
)
