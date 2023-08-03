#!/usr/bin/env bash
# Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

set -e

CAPI_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
header_file_backup="$CAPI_DIR/include/minitrace_c.h.backup"

function cleanup {
    rm -rf "$WORK_DIR" || true
    rm "$header_file_backup" || true
}

trap cleanup EXIT

WORK_DIR=$(mktemp -d)

# check if tmp dir was created
if [[ ! "$WORK_DIR" || ! -d "$WORK_DIR" ]]; then
    echo "Could not create temp dir"
    exit 1
fi

cp "$CAPI_DIR/include/minitrace_c.h" "$header_file_backup"

if ! cargo expand 2>$WORK_DIR/expand_stderr.err >$WORK_DIR/expanded.rs; then
    cat $WORK_DIR/expand_stderr.err
fi

# Bindgen!
if ! cbindgen \
    --config "$CAPI_DIR/cbindgen.toml" \
    --lockfile "$CAPI_DIR/Cargo.lock" \
    --output "$CAPI_DIR/include/minitrace_c.h" \
    "${@}" \
    $WORK_DIR/expanded.rs 2>$WORK_DIR/cbindgen_stderr.err; then
    bindgen_exit_code=$?
    if [[ "--verify" == "$1" ]]; then
        echo "Changes from previous header (old < > new)"
        diff -u "$header_file_backup" "$CAPI_DIR/include/minitrace.h"
    else
        echo "cbindgen failed:"
        cat $WORK_DIR/cbindgen_stderr.err
    fi
    exit $bindgen_exit_code
fi

exit 0
