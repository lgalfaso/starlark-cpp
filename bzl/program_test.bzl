# Copyright 2026 Lucas Mirelmann

"""Run a program as many Bazel tests with different arguments."""

load("@rules_shell//shell:sh_test.bzl", "sh_test")

_RUNNER = "//bzl:run_program_test.sh"

def program_test(name, program, args = [], data = [], size = None, **kwargs):
    """Run `program` as a Bazel test with the given command-line arguments.

    Args:
        name: Name of the generated test target.
        program: Label of an executable target to run.
        args: Arguments passed to the program. Use `$(location ...)` for data
            files listed in `data`.
        data: Runtime data dependencies. `program` is added automatically.
        size: Optional Bazel test size (`small`, `medium`, `large`, or
            `enormous`). Defaults to the `sh_test` rule default.
        **kwargs: Additional attributes forwarded to `sh_test`.
    """
    attrs = {
        "name": name,
        "srcs": [_RUNNER],
        "args": ["$(rootpath " + program + ")"] + args,
        "data": data + [program],
    }
    if size != None:
        attrs["size"] = size
    attrs.update(kwargs)
    sh_test(**attrs)
