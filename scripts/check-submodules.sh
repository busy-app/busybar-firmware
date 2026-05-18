#!/usr/bin/env bash

set -e

EXPECTED_BRANCHES="assets/proto (remotes/origin/HEAD)
fbt_layers/core_libs (heads/bsb-2-g29c0c15)
fbt_layers/fbtng (remotes/origin/bsb)
fbt_layers/freertos (heads/dev)
lib/cjson (v1.7.15-44-g12c4bf1)
lib/heatshrink (heads/master)
lib/lvgl (remotes/origin/flipper_tweaks)
lib/lwip (remotes/origin/bsb)
lib/matter_ext (v1.0.5-16-gc880f40e1)
lib/matter_zap (heads/dev)
lib/mbedtls (v3.6.4)
lib/microtar (v0.1.0-48-g1e92136)
lib/mongoose (7.20-187-gb1c2ffe1)
lib/nanopb (nanopb-0.4.9.1)
lib/simplicity_sdk (heads/sisdk-2025.12.2)
lib/stb/stb_repo (f1c79c0)
lib/thorvg (v1.0-pre32-63-g9a5112d8)
lib/tinyusb (0.16.0-3142-g405dadecc)
lib/wiseconnect (heads/bsb)
lib/zlib (v1.3.1.2-41-gedd8895)
scripts/toolchain (heads/dev)"

ACTUAL_BRANCHES=$(git submodule status | sed "s/^ *//" | cut -d ' ' -f 2,3)

if [ "$EXPECTED_BRANCHES" == "$ACTUAL_BRANCHES" ]; then
    echo "All submodules are on the correct branch"
else
    echo "Not all modules are on the correct branch:"
    diff --color <(echo "$ACTUAL_BRANCHES") <(echo "$EXPECTED_BRANCHES") || true
    echo ""
    echo "Please point these submodules to the correct branches:"
    echo ""
    echo "    cd [submodule path here]"
    echo "    git reset --hard HEAD"
    echo "    git switch --detach [branch name here]"
    exit 1
fi
