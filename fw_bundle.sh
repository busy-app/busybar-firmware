#!/usr/bin/env bash

./fbt TARGET_HW=20

mkdir -p ../fpt-bsb/assets/fw_u5
cp -v ./fbt_layers/fbtng/build/f20-firmware-D/firmware* ../fpt-bsb/assets/fw_u5


./fbt TARGET_HW=64
mkdir -p ../fpt-bsb/assets/fw_917
cp -v ./fbt_layers/fbtng/build/f64-firmware-D/firmware.rps ../fpt-bsb/assets/fw_917
cp -v ./lib/wiseconnect/connectivity_firmware/standard/SiWG917-B.2.13.4.1.0.4.rps ../fpt-bsb/assets/fw_917

