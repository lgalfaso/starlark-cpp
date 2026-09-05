# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library")

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
    srcs = ["test_case.cpp"],
    hdrs = ["test_case.hpp"],
    testonly = True,
    visibility = ["//visibility:public"],
    deps = [
        ":module_loader",
    ],
)

cc_library(
    name = "test_assertions",
    srcs = ["test_assertions.cpp"],
    hdrs = ["test_assertions.hpp"],
    testonly = True,
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
    name = "module_loader",
    srcs = ["module_loader.cpp"],
    hdrs = [
        "module_loader.hpp",
        "builtin.star",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":built_in_functions",
        ":frame",
        "//proto:starlark_bytecode_cc_proto",
        "//runtime:starlark_function",
        "//runtime:starlark_object",
        "//status_or:status",
    ],
)
