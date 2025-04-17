#!/usr/bin/env bash

./fbt TARGET_HW=64

DEVICE_IP="10.0.4.20"
DEVICE_IP_REF="10.0.4.22"

if [[ $1 != "" ]]; then
    DEVICE_IP=$1
fi

if [[ $DEVICE_IP == "ref" ]]; then
    DEVICE_IP=$DEVICE_IP_REF
fi

. ./funcs.sh
wait_for_device $DEVICE_IP

set -e -x

time ./scripts/update.py -p $DEVICE_IP 917 fbt_layers/fbtng/build/f64-firmware-D/firmware.rps
# real    0m13.607s
# real    0m10.450s, 10, 10, 10, 10, 8.8, 8.5 ref

time ./scripts/update.py -p $DEVICE_IP 917 --nwp ./lib/wiseconnect/connectivity_firmware/standard/SiWG917-B.2.13.4.1.0.4.rps
# real    1m10.021s
# real    1m31.353s, 51, 60+22, 60+45, 51, 60+20, 60+18 ref, 1m47.392s

