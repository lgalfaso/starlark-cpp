#!/usr/bin/env bash
# rules_fuzzing builds fuzz binaries with --dynamic_mode=off. On macOS, ASan is a
# dylib resolved via an rpath relative to the workspace bazel-out tree. Fuzz
# launchers often run with cwd under *.runfiles/_main, so link bazel-out there.
set -euo pipefail

if [[ "$(uname -s)" != Darwin ]]; then
  exec "$@"
fi

runfiles="${RUNFILES_DIR:-${TEST_SRCDIR:-}}"
dylib=""
bazel_out=""
candidate=""
dir=""

if [[ -n "${runfiles}" ]]; then
  dylib="$(find "${runfiles}" -name libclang_rt.asan_osx_dynamic.dylib 2>/dev/null | head -1)"
fi

if [[ -z "${dylib}" ]]; then
  for dir in "${BUILD_WORKSPACE_DIRECTORY:+${BUILD_WORKSPACE_DIRECTORY}/bazel-out}" \
             "${TEST_SRCDIR:-}" \
             "${RUNFILES_DIR:-}" \
             "$(pwd)"; do
    [[ -z "${dir}" || ! -d "${dir}" ]] && continue
    candidate="${dir}"
    while [[ "${candidate}" != "/" ]]; do
      if [[ "$(basename "${candidate}")" == "bazel-out" && -d "${candidate}" ]]; then
        bazel_out="${candidate}"
        break 2
      fi
      candidate="$(dirname "${candidate}")"
    done
  done

  if [[ -n "${bazel_out}" ]]; then
    if [[ ! -e "./bazel-out" ]]; then
      ln -s "${bazel_out}" "./bazel-out"
    fi
    dylib="$(find "${bazel_out}" -name libclang_rt.asan_osx_dynamic.dylib \
      -path '*/toolchain/*_cc_toolchain_resource_directory/*' 2>/dev/null | head -1)"
  fi
fi

if [[ -n "${dylib}" ]]; then
  export DYLD_LIBRARY_PATH="${dylib%/*}${DYLD_LIBRARY_PATH:+:${DYLD_LIBRARY_PATH}}"
fi

exec "$@"
