#!/usr/bin/env bash
# Copyright 2026 Lucas Mirelmann

set -euo pipefail

if [[ -n "${TEST_SRCDIR:-}" ]]; then
  cd "${TEST_SRCDIR}/${TEST_WORKSPACE}"
fi

program="$1"
shift
exec "./${program}" "$@"
