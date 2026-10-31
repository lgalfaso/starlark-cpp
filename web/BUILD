# Copyright 2026 Lucas Mirelmann

load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@emsdk//emscripten_toolchain:wasm_rules.bzl", "wasm_cc_binary")

package(default_visibility = ["//visibility:public"])

_WASM_LINKOPTS = [
    "-sMODULARIZE=1",
    "-sEXPORT_NAME=createStarlarkModule",
    "-sENVIRONMENT=web",
    "-sALLOW_MEMORY_GROWTH=1",
    "-sFILESYSTEM=0",
    "-sEXPORTED_FUNCTIONS=['_starlark_clear_modules','_starlark_define_module','_starlark_run','_malloc','_free']",
    "-sEXPORTED_RUNTIME_METHODS=['UTF8ToString','stringToUTF8','lengthBytesUTF8']",
]

cc_binary(
    name = "starlark_wasm",
    srcs = ["starlark_wasm_api.cpp"],
    linkopts = _WASM_LINKOPTS,
    target_compatible_with = [
        "@platforms//cpu:wasm32",
    ],
    visibility = ["//visibility:private"],
    deps = [
        "//interpreter:interpreter",
        "//logging",
    ],
)

wasm_cc_binary(
    name = "starlark_wasm_pkg",
    cc_target = ":starlark_wasm",
)

# Build this (or //web:dist), not //web:starlark_wasm — the cc_binary is Emscripten-only.
alias(
    name = "wasm",
    actual = ":starlark_wasm_pkg",
)

genrule(
    name = "dist",
    srcs = [
        "index.html",
        "playground.js",
        "playground.css",
        ":starlark_wasm_pkg",
    ],
    outs = [
        "dist/index.html",
        "dist/playground.js",
        "dist/playground.css",
        "dist/starlark_wasm.js",
        "dist/starlark_wasm.wasm",
    ],
    cmd = """
mkdir -p $$(dirname $(location dist/index.html))
cp $(location index.html) $(location dist/index.html)
cp $(location playground.js) $(location dist/playground.js)
cp $(location playground.css) $(location dist/playground.css)
for f in $(locations :starlark_wasm_pkg); do
  case "$$f" in
    *starlark_wasm.js) cp "$$f" $(location dist/starlark_wasm.js) ;;
    *starlark_wasm.wasm) cp "$$f" $(location dist/starlark_wasm.wasm) ;;
  esac
done
""",
)
