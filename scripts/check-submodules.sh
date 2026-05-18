#!/usr/bin/env bash

set -e

EXPECTED_BRANCHES="assets/proto origin/dev
fbt_layers/core_libs origin/bsb
fbt_layers/fbtng origin/bsb
fbt_layers/freertos origin/dev
lib/cjson origin/master
lib/heatshrink origin/master
lib/lvgl origin/flipper_tweaks
lib/lwip origin/bsb
lib/matter_ext origin/main
lib/matter_zap origin/dev
lib/mbedtls origin/mbedtls-3.6
lib/microtar origin/master
lib/mongoose origin/master
lib/nanopb origin/maintenance_0.4
lib/simplicity_sdk origin/sisdk-2025.12.3
lib/stb/stb_repo origin/master
lib/thorvg origin/main
lib/tinyusb origin/ncm_packet_filter
lib/wiseconnect origin/bsb
lib/zlib origin/master
scripts/toolchain origin/dev"

TABLE_FILE=$(mktemp "/tmp/flipper-bsb-chk-submodules.XXXXXXXX")
echo "$EXPECTED_BRANCHES" > $TABLE_FILE

if git submodule foreach 'bash $toplevel/scripts/check-submodule.sh '"$TABLE_FILE"' $sm_path'; then
    echo "All submodules are on the correct branch."
else
    echo "Not all modules are on the correct branch. See output above."
    exit 1
fi

rm $TABLE_FILE
