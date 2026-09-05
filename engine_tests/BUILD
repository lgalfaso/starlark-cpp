# Copyright 2026 Lucas Mirelmann

load("//bzl:engine_tests.bzl", "engine_program_tests")

_STARS = glob(["*.star"])

_STAR_LABELS = ["//engine_tests:" + star for star in _STARS]

filegroup(
    name = "test_sources",
    srcs = _STARS,
    visibility = ["//visibility:public"],
)

engine_program_tests(
    name_prefix = "interpreter_test",
    program = "//interpreter:interpreter_test",
    star_labels = _STAR_LABELS,
)

test_suite(
    name = "interpreter_tests",
    tests = ["interpreter_test_" + star.removesuffix(".star") for star in _STARS],
)
