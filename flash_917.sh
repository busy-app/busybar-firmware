#!/usr/bin/env bash

# Ensure 917 in boot mode
# Check fbt_options_local.py
# SI917_PORT=/dev/cu.usbmodem2122201

# (venv) lomalkin@mbp16 ~/_repoz/bsb-firmware % ls -lha /dev/cu.usbmodem*                                   
# crw-rw-rw-  1 root  wheel  0x9000009 Mar 18 20:42 /dev/cu.usbmodem21224101
# crw-rw-rw-  1 root  wheel  0x9000007 Mar 18 20:42 /dev/cu.usbmodem614_id1001

SI917_PORT=$(ls /dev/cu.usbmodem* | head -n 1)

echo "SI917_PORT: $SI917_PORT"

set -e -x

./fbt TARGET_HW=64 SI917_PORT=$SI917_PORT $* flash
