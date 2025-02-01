# Copyright 2024-2025 Lucas Mirelmann

def _output_name(f, prefix):
  return f.short_path.removeprefix(prefix).removeprefix("/").removesuffix(".txtpb") + ".binpb"

def _starlark_proto_encoder_rule_impl(ctx):
    outputs = []
    for f in ctx.attr.input:
        for ff in f.files.to_list():
            output_file = ctx.actions.declare_file(_output_name(ff, ctx.attr.strip_prefix))
            args = ctx.actions.args()
            args.add(ff)
            args.add(output_file)
            ctx.actions.run(
                mnemonic = "ProtoEncoder",
                executable = ctx.executable._encoder,
                arguments = [args],
                inputs = [ff],
                outputs = [output_file],
            )
            outputs.append(output_file)
    return DefaultInfo(files = depset(outputs))


_starlark_proto_encoder_rule = rule(
    implementation = _starlark_proto_encoder_rule_impl,
    attrs = {
        "input": attr.label_list(
            allow_files = [".txtpb"],
        ),
        "strip_prefix": attr.string(),
        "_encoder": attr.label(
            default = Label("//grammar:parser_proto_encoder"),
            allow_single_file = True,
            executable = True,
            cfg = "exec",
        ),
    },
)

def starlark_proto_encoder(**kwargs):
    if "strip_prefix" in kwargs:
        fail("Cannot have the argument `strip_prefix`")
    _starlark_proto_encoder_rule(
        strip_prefix = native.package_name(),
        **kwargs,
    )
