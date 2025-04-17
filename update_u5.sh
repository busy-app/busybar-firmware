#!/usr/bin/env bash

./fbt TARGET_HW=20

DEVICE_IP="10.0.4.20"
DEVICE_IP_REF="10.0.4.22"

if [[ $1 != "" ]]; then
    DEVICE_IP=$1
fi

if [[ $DEVICE_IP == "ref" ]]; then
    DEVICE_IP=$DEVICE_IP_REF
fi

. ./funcs.sh
# wait_for_device $DEVICE_IP

set -e -x

time ./scripts/update.py -p $DEVICE_IP u5 fbt_layers/fbtng/build/f20-firmware-D/firmware.dfu --to-dfu
# time ./scripts/update.py -p $DEVICE_IP u5 fbt_layers/fbtng/build/f20-firmware-D/firmware.dfu
# real    0m46.116s, 45, 45, 45, 42, 42

# Fallback direct:
# time ./scripts/update.py u5 fbt_layers/fbtng/build/f20-firmware-D/firmware.dfu





# dfu-util -a 0 -D fbt_layers/fbtng/build/f20-firmware-D/firmware.dfu -R

