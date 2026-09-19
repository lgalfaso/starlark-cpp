load("//bzl:program_test.bzl", "program_test")

# Megatests excluded from the native spec suite: the JIT path is an order of
# magnitude slower on these files (~60k+ assertions each). They remain covered
# by spec_interpreter_test_* targets and the corpus filegroup exclusion.
_HEAVY_SPEC_TESTS = [
    "int_xor.star",
    "int_and.star",
    "int_or.star",
    "range_slice.star",
]

[
    program_test(
        name = "spec_interpreter_test_" + f.replace("/", "_"),
        args = [
            "$(location " + f + ")",
        ],
        data = [
            f,
        ],
        program = "//spec:spec_interpreter_test",
        tags = ["spec"],
    )
    for f in glob(["*.star"])
]

[
    program_test(
        name = "spec_native_test_" + f.replace("/", "_"),
        args = [
            "$(location " + f + ")",
        ],
        data = [
            f,
        ],
        program = "//spec:spec_native_test",
        tags = ["spec"],
    )
    for f in glob(
        ["*.star"],
        exclude = _HEAVY_SPEC_TESTS,
    )
]

test_suite(
    name = "all_native_spec_tests",
    tags = [
        "native",
        "spec",
    ],
    tests = [":spec_native_test_" + f.replace("/", "_") for f in glob(
        ["*.star"],
        exclude = _HEAVY_SPEC_TESTS,
    )],
)

filegroup(
    name = "corpus_filegroup",
    srcs = glob(
        ["*.star"],
        exclude = _HEAVY_SPEC_TESTS,
    ),
    visibility = ["//visibility:public"],
)
