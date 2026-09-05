# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//bzl:program_test.bzl", "program_test")

cc_binary(
    name = "errors_interpreter_test",
    srcs = ["errors_interpreter_test.cpp"],
    testonly = True,
    deps = [
        "//grammar:parsing_options",
        "//interpreter:interpreter",
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
        program = ":errors_interpreter_test",
        size = "small",
        args = [
            "$(location " + f + ")",
            "$(location " + f.replace(".star", ".error") + ")",
        ],
        data = [
            f,
            f.replace(".star", ".error"),
        ],
    )
    for f in glob(["*.star"])
]

filegroup(
    name = "corpus_filegroup",
    srcs = glob(["*.star"]),
    visibility = ["//visibility:public"],
)
