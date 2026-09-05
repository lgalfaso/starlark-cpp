load("//bzl:program_test.bzl", "program_test")

_HEAVY_SPEC_TESTS = [
    "int_xor.star",
    "int_and.star",
    "int_or.star",
    "range_slice.star",
]

[
    program_test(
        name = "spec_interpreter_test_" + f.replace("/", "_"),
        program = "//spec:spec_interpreter_test",
        args = [
            "$(location " + f + ")",
        ],
        data = [
            f,
        ],
        tags = ["spec"],
    ) for f in glob(["*.star"])
]

filegroup(
    name = "corpus_filegroup",
    srcs = glob(["*.star"], exclude = _HEAVY_SPEC_TESTS),
    visibility = ["//visibility:public"],
)
