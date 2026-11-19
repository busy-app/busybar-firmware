#!/bin/bash
set -o errexit
set -o nounset
set -o pipefail

TEST_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$TEST_DIR/../../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/.cache/brightness-control-tests}"
mkdir "$ROOT/.cache" 2>/dev/null || test -d "$ROOT/.cache"
mkdir "$BUILD_DIR" 2>/dev/null || test -d "$BUILD_DIR"

for STATUS in without_status with_status; do
    DEFINES=(--define-macro=SRV_LIGHT_SENSOR)
    if [[ "$STATUS" == with_status ]]; then
        DEFINES+=(--define-macro=SRV_STATUS_LIGHTS)
    fi
    "${CC:-clang}" --std=c11 --debug -Wall -Wextra -Werror \
        -fsanitize=address,undefined -fno-omit-frame-pointer "${DEFINES[@]}" \
        --include-directory="$TEST_DIR/shims" \
        --include-directory="$ROOT/applications/services" \
        --include-directory="$ROOT/applications/services/brightness_control" \
        "$TEST_DIR/brightness_control_host_test.c" \
        "$ROOT/applications/services/brightness_control/brightness_conv.c" \
        --output="$BUILD_DIR/$STATUS"
    "$BUILD_DIR/$STATUS"
done
