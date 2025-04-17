#!/usr/bin/env bash

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

time ./scripts/storage.py -p $DEVICE_IP send ../bsb-firmware/assets/audio /ext/audio

