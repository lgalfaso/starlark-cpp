# Differential engine testing

This package runs the same Starlark program on the **interpreter** and **native** engines and checks that they agree.

## What is compared

- **Failure vs success:** `status_code` (ok, static error, runtime error) must match.
- **On success:** exported module globals (names not starting with `_`), compared by `repr()`; functions are compared only as the placeholder `"<function>"`.

Fuzz and `differential_check` use the same `assert_*` predeclared bindings as spec/engine tests (`check_source_like_spec_tests`).

Multi-module inputs use the `## Begin module "name"` … `## End module` format (see `vm/multi_module_source`).

## Quick checks

```bash
# GTest smoke tests (interpreter + native on a few fixed programs)
bazel test //differential:engine_test

# Compare both engines on one file (prints "ok" or a mismatch reason)
bazel run //differential:differential_check -- path/to/file.star
```

## Fuzzing (libFuzzer)

Fuzz targets use the hermetic toolchain and `--config=asan-libfuzzer` (see `rules_fuzzer.bazelrc`).

Each fuzz **input** is a byte string treated as Starlark source. The harness runs **interpreter and native** on every non-empty input (up to 64 KiB). Seeds come from lexer/parser/error/spec corpora declared in `BUILD`.

### Short smoke run (CI-friendly)

`bazel test` runs a **single** fuzzer process. Cap work with **libFuzzer** flags via `--test_arg` (these are forwarded to the instrumented binary, not the rules_fuzzing launcher):

```bash
bazel test //differential:fuzz_test --config=asan-libfuzzer \
  --test_arg=-runs=100 \
  --test_arg=-max_len=4096
```

Or a time budget (seconds):

```bash
bazel test //differential:fuzz_test --config=asan-libfuzzer \
  --test_timeout=3600 \
  --test_arg=-max_total_time=3300 \
  --test_arg=-timeout=10 \
  --test_arg=-max_len=8192
```

Use `max_total_time` slightly below Bazel’s `test_timeout` so the run exits cleanly.

| Flag | Unit | Meaning |
|------|------|--------|
| `-runs=N` | count | Stop after N executions (see note below on seed corpus). |
| `-max_total_time=S` | **seconds** | Stop after S seconds of wall time. `0` = no limit (run until stopped). |
| `-timeout=T` | seconds | Per-input execution limit (raise for slow native JIT). |
| `-max_len=N` | bytes | Max input size (harness also skips inputs over 64 KiB). |

Without `-runs` or `-max_total_time`, libFuzzer keeps going until a crash or the Bazel test timeout.

`-runs=N` is a ceiling on **total** executions; libFuzzer still replays every file in the bundled seed corpus once at startup, so smoke runs with a large seed tree may do more than `N` individual runs if `N` is below the seed file count.

### Continuous / long-running fuzz

Two entry points exist:

| Target | What accepts CLI flags |
|--------|-------------------------|
| `//differential:fuzz_test_bin` | **libFuzzer** (`-max_total_time`, `-timeout`, `-max_len`, …) |
| `//differential:fuzz_test_run` | **rules_fuzzing launcher** (`--timeout_secs`, `--fuzzing_output_root`, `--clean`; run `-- --helpshort`) |

**Do not** pass libFuzzer `-flag=value` arguments to `fuzz_test_run`. Bazel forwards them to the Python launcher, which rejects unknown flags (for example `Unknown command line flag 'max_total_time'`).

For full libFuzzer control and a corpus directory you own, run the instrumented binary directly:

```bash
CORPUS="${HOME}/starlark-differential-fuzz-corpus"
mkdir -p "${CORPUS}"

bazel run --config=asan-libfuzzer -c opt //differential:fuzz_test_bin -- \
  -max_total_time=0 \
  -timeout=10 \
  -max_len=8192 \
  -artifact_prefix=/tmp/differential-fuzz-crash- \
  -dict=differential/fuzz_test.dict \
  "${CORPUS}" \
  differential/fuzz_test_corpus
```

- Put **libFuzzer flags before corpus paths** (the harness inserts defaults in `LLVMFuzzerInitialize` before trailing argv entries).
- The first directory (`"${CORPUS}"`) is your writable on-disk corpus; `differential/fuzz_test_corpus` is the read-only Bazel seed corpus (path is relative to the runfiles dir that `bazel run` uses).
- Run under `tmux`, `nohup`, or a scheduler for overnight soaks.
- On mismatch or crash, the harness traps; use `-artifact_prefix` to save reproducers and minimize with libFuzzer if needed.

**Convenience wrapper** (`fuzz_test_run`): seeds, dictionary, and artifact paths are prewired; new inputs go under `/tmp/fuzzing/corpus` unless you set `--fuzzing_output_root`. Example time-bounded soak (launcher flag, not libFuzzer):

```bash
bazel run --config=asan-libfuzzer -c opt //differential:fuzz_test_run -- \
  --fuzzing_output_root="${HOME}/starlark-differential-fuzz" \
  --timeout_secs=0
```

`--timeout_secs=0` (default) means no wall-clock limit; omitting it is the same as open-ended fuzz until you stop the process.

### Memory (ASAN + native JIT)

The differential fuzz binary links LLVM and runs JIT codegen per interesting input, so libFuzzer’s default **2048 MiB** RSS cap is easy to hit. For libFuzzer runs (`fuzz_test_bin` / OSS-Fuzz), the harness sets **`LLVMFuzzerInitialize` defaults** unless you override them on the command line. Bazel’s seed-corpus regression test (`bazel test //differential:fuzz_test`) uses the replay engine and skips argv injection so corpus paths are not mistaken for inputs.

| Flag | Default injected | Why |
|------|------------------|-----|
| `-use_value_profile=0` | yes | Value profiling keeps a large per-edge feature set; LLVM JIT already provides huge coverage. |
| `-rss_limit_mb=8192` | yes | ASAN quarantine + in-memory corpus + JIT peaks need more than 2 GiB on long soaks. |

Native runs use **`fast_jit`** (LLVM `CodeGenOptLevel::Default`) and the same runtime size limits as `engine_test`. Override RSS with `-rss_limit_mb=0` (no limit) or a higher value if needed.

## When a mismatch is found

1. stderr prints the reason and the source bytes.
2. Confirm with `bazel run //differential:differential_check -- repro.star`.
3. Turn a minimized input into a regression under `engine_tests/` or `spec/tests/` as appropriate.

## Library targets

| Target | Role |
|--------|------|
| `//differential:engine` | Run/compare API (`starlark::differential`). |
| `//differential:spec_support` | Spec-style `assert_*` bindings wrapper. |
