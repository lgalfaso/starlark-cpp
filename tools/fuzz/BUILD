# Copyright 2026 Lucas Mirelmann

load("@rules_shell//shell:sh_binary.bzl", "sh_binary")

package(default_visibility = ["//visibility:public"])

sh_binary(
    name = "macos_asan_run_under",
    srcs = ["macos_asan_run_under.sh"],
    data = ["@llvm//runtimes/compiler-rt:clang_rt.asan.shared"],
)
