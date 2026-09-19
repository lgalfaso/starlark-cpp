# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//native:__subpackages__"])

cc_library(
    name = "jit_ctx_layout",
    hdrs = ["jit_ctx_layout.hpp"],
    deps = ["//runtime:starlark_object"],
)

cc_library(
    name = "jit_stack",
    srcs = ["jit_stack.cpp"],
    hdrs = ["jit_stack.hpp"],
    deps = [
        "//runtime:starlark_object",
        "//vm:frame",
    ],
)

cc_library(
    name = "native_exec_context",
    srcs = ["native_exec_context.cpp"],
    hdrs = ["native_exec_context.hpp"],
    deps = [
        ":jit_stack",
        "//native/abi:starlark_module_abi",
        "//runtime:starlark_object",
        "//vm:frame",
    ],
)

cc_library(
    name = "module_runtime_state",
    srcs = ["module_runtime_state.cpp"],
    hdrs = ["module_runtime_state.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":native_exec_context",
        "//proto:starlark_bytecode_cc_proto",
        "//runtime:options",
        "//runtime:starlark_object",
        "//runtime:starlark_string",
        "//vm:frame",
        "//vm:module_loader",
        "//vm:module_metadata",
        "@protobuf",
    ],
)

cc_library(
    name = "jit_exec_layout",
    srcs = ["jit_exec_layout.cpp"],
    hdrs = ["jit_exec_layout.hpp"],
    deps = [
        ":module_runtime_state",
        ":native_exec_context",
        "//vm:frame",
    ],
)

cc_library(
    name = "frame_setup_cache",
    srcs = ["frame_setup_cache.cpp"],
    hdrs = ["frame_setup_cache.hpp"],
    deps = [
        ":native_exec_context",
        ":module_runtime_state",
        "//runtime:starlark_object",
        "//vm:frame_factory",
        "//vm:module_metadata",
        "@protobuf",
    ],
)

cc_library(
    name = "exec_error",
    srcs = ["exec_error.cpp"],
    hdrs = ["exec_error.hpp"],
    deps = [
        ":native_exec_context",
        ":module_runtime_state",
        "//proto:starlark_logging_cc_proto",
        "//runtime:starlark_object",
    ],
)

cc_library(
    name = "runtime_shim",
    srcs = ["runtime_shim.cpp"],
    hdrs = ["runtime_shim.hpp"],
    deps = [
        ":exec_error",
        ":frame_setup_cache",
        ":native_exec_context",
        "//errors:runtime_error_messages",
        ":module_runtime_state",
        "//runtime:starlark_function",
        "//runtime:starlark_object",
        "//vm:frame",
        "//vm:frame_factory",
        "//vm:module_loader",
        "//vm:module_metadata",
        "//vm:predeclared_context",
        "@protobuf",
    ],
    alwayslink = True,
)

cc_library(
    name = "native_function",
    srcs = ["native_function.cpp"],
    hdrs = ["native_function.hpp"],
    deps = [
        ":exec_error",
        ":frame_setup_cache",
        ":native_exec_context",
        ":runtime_shim",
        "//errors:runtime_error_messages",
        "//native/abi:starlark_module_abi",
        ":module_runtime_state",
        "//proto:starlark_bytecode_cc_proto",
        "//runtime:starlark_function",
        "//vm:frame",
        "//vm:frame_factory",
        "//vm:function_call_binding",
        "//vm:module_metadata",
        "@protobuf",
    ],
)

cc_library(
    name = "object_runtime",
    srcs = ["object_runtime.cpp"],
    hdrs = ["object_runtime.hpp"],
    deps = [
        ":exec_error",
        ":native_exec_context",
        ":native_function",
        "//bigint:number",
        "//errors:runtime_error_messages",
        "//native/abi:starlark_module_abi",
        ":module_runtime_state",
        "//proto:starlark_logging_cc_proto",
        "//runtime:starlark_bigint",
        "//runtime:starlark_bytes",
        "//runtime:starlark_dictionary",
        "//runtime:starlark_float",
        "//runtime:starlark_list",
        "//runtime:starlark_numeric_impl",
        "//runtime:starlark_object",
        "//runtime:starlark_string",
        "//runtime:starlark_tuple",
        "//vm:fast_compare",
        "//vm:frame",
        "//vm:frame_factory",
        "@protobuf",
    ],
    alwayslink = True,
)

cc_test(
    name = "jit_exec_layout_test",
    srcs = ["jit_exec_layout_test.cpp"],
    deps = [
        ":jit_exec_layout",
        "@googletest//:gtest_main",
    ],
)
