load("@rules_cc//cc:defs.bzl", "cc_test")

[
    cc_test(
        name = "spec_interpreter_test_" + f.replace("/", "_"),
        args = [
            "$(location " + f + ")",
        ],
        data = [
            f,
        ],
        deps = [
            "//spec:spec_interpreter_test_base",
            "@googletest//:gtest_main",
        ],
        tags = ["spec"],
    ) for f in glob(["*.star"])
]

