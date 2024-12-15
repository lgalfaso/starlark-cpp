# Copyright 2024 Lucas Mirelmann

def _extract_ucd_impl(ctx):
    args = ctx.actions.args()
    args.add(ctx.file.derived_core_properties)
    args.add(ctx.file.unicode_data)
    args.add(ctx.file.composition_exclusions)
    args.add(ctx.file.derived_normalization_props)
    args.add(ctx.outputs.output_cc)
    args.add(ctx.outputs.output_h)
    args.add(ctx.outputs.output_h.short_path)

    ctx.actions.run(
        inputs = [
            ctx.file.derived_core_properties,
            ctx.file.unicode_data,
            ctx.file.composition_exclusions,
            ctx.file.derived_normalization_props,
        ],
        outputs = [ctx.outputs.output_cc, ctx.outputs.output_h],
        arguments = [args],
        executable = ctx.executable.gen_tool,
    )

extract_ucd = rule(
    implementation = _extract_ucd_impl,
    attrs = {
        "derived_core_properties": attr.label(allow_single_file = True, default = "@ucd//:DerivedCoreProperties.txt"),
        "unicode_data": attr.label(allow_single_file = True, default = "@ucd//:UnicodeData.txt"),
        "composition_exclusions": attr.label(allow_single_file = True, default = "@ucd//:CompositionExclusions.txt"),
        "derived_normalization_props": attr.label(allow_single_file = True, default = "@ucd//:DerivedNormalizationProps.txt"),
        "output_cc": attr.output(),
        "output_h": attr.output(),
        "gen_tool": attr.label(
            default = Label(":extract_main"),
            executable = True,
            allow_files = True,
            cfg = "exec",
        ),
    },
)
