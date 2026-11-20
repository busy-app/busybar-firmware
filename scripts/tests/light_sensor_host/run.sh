#!/bin/bash
set -o errexit -o nounset -o pipefail

ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
BUILD="${TMPDIR:-/tmp}/light-sensor-host.$$"
mkdir "$BUILD"
trap 'find "$BUILD" -depth -delete' EXIT

"${CC:-clang}" --std=gnu11 -Wall -Wextra -Werror --debug -fsanitize=address,undefined \
    -fno-omit-frame-pointer -fno-sanitize-recover=all \
    --include-directory="$ROOT/scripts/tests/light_sensor_host" \
    --include-directory="$ROOT/applications/debug/unit_tests" \
    --include-directory="$ROOT/applications/services" \
    --include-directory="$ROOT/lib/toolbox" \
    --include-directory="$ROOT/fbt_layers/core_libs/lib/furi" \
    "$ROOT/scripts/tests/light_sensor_host/light_sensor_host.c" \
    "$ROOT/applications/debug/unit_tests/light_sensor_test/light_sensor_test.c" \
    "$ROOT/applications/services/light_sensor/light_sensor_data.c" \
    -lm --output="$BUILD/light_sensor_host"

"$BUILD/light_sensor_host"
