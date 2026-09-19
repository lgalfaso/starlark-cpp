# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//native:__subpackages__"])

cc_library(
    name = "cache",
    srcs = ["cache.cpp"],
    hdrs = ["cache.hpp"],
    deps = [
        "//native/abi:starlark_module_abi",
        "//runtime:hex_format",
        "//status_or:status",
    ],
)

cc_library(
    name = "orc_engine",
    srcs = ["orc_engine.cpp"],
    hdrs = ["orc_engine.hpp"],
    tags = ["no-sandbox"],
    deps = [
        "//native/abi:starlark_module_abi",
        "//status_or:status",
        "//third-party/llvm:llvm_orc",
    ],
)

cc_test(
    name = "orc_engine_spike_test",
    srcs = ["orc_engine_spike_test.cpp"],
    deps = [
        ":orc_engine",
        "//native/ir:llvm_ir_generator",
        "//runtime:starlark_function",
        "//runtime:starlark_function_builder_impl",
        "//runtime:starlark_object_impl",
        "//status_or:status",
        "//third-party/llvm:llvm_orc",
        "//vm:module_metadata",
        "@googletest//:gtest_main",
    ],
)
