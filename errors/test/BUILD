# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//bzl:program_test.bzl", "program_test")

cc_binary(
    name = "errors_interpreter_test",
    testonly = True,
    srcs = ["errors_interpreter_test.cpp"],
    deps = [
        "//grammar:parsing_options",
        "//interpreter",
        "//interpreter:interpreter_test_runner",
        "//io:read_file",
        "//logging",
        "//runtime:parsing_options",
        "//runtime:starlark_object",
        "//vm:module_loader",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

[
    program_test(
        name = "errors_test_" + f.replace("/", "_"),
        size = "small",
        args = [
            "$(location " + f + ")",
            "$(location " + f.replace(".star", ".error") + ")",
        ],
        data = [
            f,
            f.replace(".star", ".error"),
        ],
        program = ":errors_interpreter_test",
    )
    for f in glob(["*.star"])
]

filegroup(
    name = "corpus_filegroup",
    srcs = glob(["*.star"]),
    visibility = ["//visibility:public"],
)
