# Copyright 2026 Lucas Mirelmann

load(":llvm_orc.bzl", "llvm_orc")

llvm_orc(
    name = "llvm_orc",
    tags = ["no-sandbox"],
    visibility = ["//visibility:public"],
    deps = select({
        "@platforms//cpu:arm64": [
            "@llvm-project//llvm:AArch64AsmParser",
            "@llvm-project//llvm:AArch64CodeGen",
            "@llvm-project//llvm:AArch64Info",
            "@llvm-project//llvm:AArch64UtilsAndDesc",
            "@llvm-project//llvm:OrcJIT",
        ],
        "//conditions:default": [
            "@llvm-project//llvm:OrcJIT",
            "@llvm-project//llvm:X86AsmParser",
            "@llvm-project//llvm:X86CodeGen",
            "@llvm-project//llvm:X86Info",
            "@llvm-project//llvm:X86UtilsAndDesc",
        ],
    }),
)

llvm_orc(
    name = "llvm_lld",
    tags = ["no-sandbox"],
    visibility = ["//visibility:public"],
    deps = select({
        "@platforms//os:macos": [
            "@llvm-project//lld:MachO",
            "@llvm-project//llvm:AllTargetsAsmParsers",
            "@llvm-project//llvm:AllTargetsCodeGens",
        ],
        "@platforms//os:linux": [
            "@llvm-project//lld:ELF",
            "@llvm-project//llvm:AllTargetsAsmParsers",
            "@llvm-project//llvm:AllTargetsCodeGens",
        ],
        "//conditions:default": [
            "@llvm-project//lld:ELF",
            "@llvm-project//llvm:AllTargetsAsmParsers",
            "@llvm-project//llvm:AllTargetsCodeGens",
        ],
    }),
)
