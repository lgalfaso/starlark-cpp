"""Helpers for running shared engine test programs."""

load("//bzl:program_test.bzl", "program_test")

def _basename(star_label):
    return star_label.split(":")[-1].removesuffix(".star")

def engine_program_tests(name_prefix, program, star_labels, tags = []):
    """Declare program_test targets for each Starlark test file.

    Args:
        name_prefix: Prefix for generated test target names.
        program: Label of the test binary to execute.
        star_labels: Labels of .star files, e.g. ["//engine_tests:assign.star"].
        tags: Optional tags forwarded to each program_test.
    """
    for star_label in star_labels:
        program_test(
            name = name_prefix + "_" + _basename(star_label),
            program = program,
            tags = tags,
            args = ["$(location " + star_label + ")"],
            data = [star_label],
        )
