# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load("@rules_fuzzing//fuzzing:cc_defs.bzl", "cc_fuzz_test")

package(default_visibility = ["//native:__subpackages__"])

cc_library(
    name = "native_options",
    hdrs = ["native_options.hpp"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "runner_state",
    hdrs = ["runner_state.hpp"],
    deps = [
        ":native_options",
        "//vm:module_loader",
    ],
)

cc_library(
    name = "native_runtime",
    srcs = ["native_runtime.cpp"],
    hdrs = ["native_runtime.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//native/engine:orc_engine",
        "//native/exec:native_exec_context",
        "//native/exec:module_runtime_state",
    ],
)

cc_library(
    name = "native_runner",
    srcs = ["native_runner.cpp"],
    hdrs = ["native_runner.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":native_options",
        ":native_runtime",
        ":runner_state",
        "//compiler",
        "//errors:source_highlight",
        "//grammar:options",
        "//logging",
        "//native/abi:starlark_module_abi",
        "//native/engine:cache",
        "//native/exec:exec_error",
        "//native/exec:object_runtime",
        "//native/exec:runtime_shim",
        "//native/ir:llvm_ir_generator",
        "//native/exec:module_runtime_state",
        "//proto:starlark_bytecode_cc_proto",
        "//runtime:options",
        "//runtime:starlark_object",
        "//status_or:status",
        "//third-party/llvm:llvm_orc",
        "//vm:module_load_tracker",
        "//vm:module_loader",
        "//vm:module_metadata",
        "//vm:program_limits",
    ],
)

cc_binary(
    name = "native_main",
    srcs = ["native_main.cpp"],
    deps = [
        ":native_runner",
        "//io:read_file",
    ],
)

cc_library(
    name = "native_test_runner",
    testonly = True,
    srcs = ["native_test_runner.cpp"],
    hdrs = ["native_test_runner.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":native_options",
        ":native_runner",
        ":runner_state",
        "//grammar:options",
        "//logging",
        "//runtime:starlark_function",
        "//runtime:starlark_types",
        "//vm:test_assertions",
        "//vm:test_case",
        "@protobuf",
    ],
)

cc_binary(
    name = "native_test",
    testonly = True,
    srcs = ["native_test.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":native_test_runner",
        "//io:read_file",
        "//vm:test_case",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_fuzz_test(
    name = "native_fuzz_test",
    srcs = ["native_fuzz_test.cpp"],
    corpus = [
        "//errors/test:corpus_filegroup",
        "//grammar/lexer_fuzz_test_corpus:corpus_filegroup",
        "//spec/tests:corpus_filegroup",
    ],
    dicts = [
        "//grammar/lexer_fuzz_test_dictionary:lexical_elements.dict",
    ],
    deps = [
        ":native_runner",
    ],
)
