# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//native:__subpackages__"])

cc_library(
    name = "irgen_options",
    hdrs = ["irgen_options.hpp"],
    deps = ["//vm:module_metadata"],
)

cc_library(
    name = "ir_exec_context",
    srcs = ["ir_exec_context.cpp"],
    hdrs = ["ir_exec_context.hpp"],
    deps = [
        "//native/exec:jit_ctx_layout",
        "//native/exec:jit_exec_layout",
        "//native/exec:native_exec_context",
        "//native/exec:object_runtime",
        "//native/exec:module_runtime_state",
        "//third-party/llvm:llvm_orc",
    ],
)

cc_library(
    name = "lowering_context",
    hdrs = ["lowering_context.hpp"],
    deps = [
        ":ir_exec_context",
        ":irgen_options",
    ],
)

cc_library(
    name = "ir_object_ops",
    srcs = ["ir_object_ops.cpp"],
    hdrs = ["ir_object_ops.hpp"],
    deps = [
        ":ir_exec_context",
        ":lowering_context",
        "//native/exec:jit_exec_layout",
        "//proto:starlark_bytecode_cc_proto",
        "//runtime:starlark_object",
        "//third-party/llvm:llvm_orc",
        "//vm:module_metadata",
    ],
)

cc_library(
    name = "opcode_lowering",
    srcs = ["opcode_lowering.cpp"],
    hdrs = ["opcode_lowering.hpp"],
    deps = [
        ":ir_exec_context",
        ":ir_object_ops",
        ":irgen_options",
        ":lowering_context",
        "//proto:starlark_bytecode_cc_proto",
        "//third-party/llvm:llvm_orc",
        "//vm:module_metadata",
    ],
)

cc_library(
    name = "bytecode_lowering",
    srcs = ["bytecode_lowering.cpp"],
    hdrs = ["bytecode_lowering.hpp"],
    tags = ["no-sandbox"],
    deps = [
        ":ir_exec_context",
        ":ir_object_ops",
        ":irgen_options",
        ":lowering_context",
        ":opcode_lowering",
        "//native/abi:starlark_module_abi",
        "//proto:starlark_bytecode_cc_proto",
        "//third-party/llvm:llvm_orc",
    ],
)

cc_library(
    name = "llvm_ir_generator",
    srcs = ["llvm_ir_generator.cpp"],
    hdrs = ["llvm_ir_generator.hpp"],
    tags = ["no-sandbox"],
    deps = [
        ":bytecode_lowering",
        ":irgen_options",
        "//proto:starlark_bytecode_cc_proto",
        "//third-party/llvm:llvm_orc",
        "//vm:module_metadata",
    ],
)

cc_test(
    name = "bytecode_lowering_test",
    srcs = ["bytecode_lowering_test.cpp"],
    deps = [
        ":bytecode_lowering",
        ":llvm_ir_generator",
        "//runtime:starlark_function",
        "//runtime:starlark_function_builder_impl",
        "//runtime:starlark_object_impl",
        "//third-party/llvm:llvm_orc",
        "//vm:module_metadata",
        "@googletest//:gtest_main",
    ],
)
