"""LLVM OrcJIT dependency bundle with compile settings for @llvm-project."""

load("@rules_cc//cc/common:cc_common.bzl", "cc_common")
load("@rules_cc//cc/common:cc_info.bzl", "CcInfo")

def _llvm_orc_transition_impl(_settings, _attr):
    llvm_copts = [
        "-std=c++17",
        "-fexceptions",
        "-frtti",
        "-UGTEST_HAS_RTTI",
    ]
    return {
        "//command_line_option:cxxopt": llvm_copts,
        "//command_line_option:host_cxxopt": llvm_copts,
    }

_llvm_orc_transition = transition(
    implementation = _llvm_orc_transition_impl,
    inputs = [],
    outputs = [
        "//command_line_option:cxxopt",
        "//command_line_option:host_cxxopt",
    ],
)

def _llvm_orc_impl(ctx):
    cc_infos = [dep[CcInfo] for dep in ctx.attr.deps]
    return [cc_common.merge_cc_infos(cc_infos = cc_infos)]

llvm_orc = rule(
    implementation = _llvm_orc_impl,
    attrs = {
        "deps": attr.label_list(
            cfg = _llvm_orc_transition,
            providers = [CcInfo],
        ),
        "_allowlist_function_transition": attr.label(
            default = "@bazel_tools//tools/allowlists/function_transition_allowlist",
        ),
    },
    provides = [CcInfo],
)
