# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "error_test_base",
    srcs = ["error_test.cpp"],
    alwayslink = 1,
    testonly = True,
    deps = [
        "//grammar:parsing_options",
        "//interpreter:interpreter",
        "//interpreter:interpreter_test_runner",
        "//interpreter:module_loader",
        "//logging",
        "//runtime:parsing_options",
        "//runtime:starlark_object",
        "//third-party:defer",
        "@googletest//:gtest",
    ],
)

[
    cc_test(
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
        deps = [
            ":error_test_base",
            "@googletest//:gtest_main",
        ],
    )
    for f in glob(["*.star"])
]

filegroup(
    name = "corpus_filegroup",
    srcs = glob(["*.star"]),
    visibility = ["//visibility:public"],
)
