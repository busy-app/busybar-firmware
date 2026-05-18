#!/usr/bin/env bash

set -e

EXPECTED_BRANCHES="Entering 'assets/proto'
remotes/origin/HEAD
Entering 'fbt_layers/core_libs'
heads/bsb-2-g29c0c15
Entering 'fbt_layers/fbtng'
remotes/origin/bsb
Entering 'fbt_layers/freertos'
heads/dev
Entering 'lib/cjson'
tags/v1.7.18-11-g12c4bf1
Entering 'lib/heatshrink'
heads/master
Entering 'lib/lvgl'
remotes/origin/flipper_tweaks
Entering 'lib/lwip'
remotes/origin/bsb
Entering 'lib/matter_ext'
heads/main
Entering 'lib/matter_zap'
heads/dev
Entering 'lib/mbedtls'
tags/v3.6.4
Entering 'lib/microtar'
heads/master
Entering 'lib/mongoose'
tags/7.21
Entering 'lib/nanopb'
tags/nanopb-0.4.9.1
Entering 'lib/simplicity_sdk'
heads/sisdk-2025.12.2
Entering 'lib/stb/stb_repo'
f1c79c0
Entering 'lib/thorvg'
tags/v1.0-pre32-63-g9a5112d8
Entering 'lib/tinyusb'
remotes/origin/ncm_packet_filter
Entering 'lib/wiseconnect'
heads/bsb
Entering 'lib/zlib'
tags/v1.3.1.2-41-gedd8895
Entering 'scripts/toolchain'
heads/dev"

ACTUAL_BRANCHES=$(git submodule foreach git describe --all --always)

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
