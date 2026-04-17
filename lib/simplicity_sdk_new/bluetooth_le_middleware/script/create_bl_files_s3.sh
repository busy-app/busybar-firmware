#!/bin/bash

################################################################################
#
# Copyright 2024 Silicon Laboratories Inc. www.silabs.com
#
# SPDX-License-Identifier: Zlib
#
# The licensor of this software is Silicon Laboratories Inc.
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#
################################################################################
#
# GBL file generator for Series 3 devices
#
# This script generates GBL4 (Gecko Bootloader version 4) file out of a project
# build artifact (s37). The GBL4 format is compatible with Series 3 devices.
# It can be used with Application OTA DFU (over-the-air device firmware update).
# Note that the previous format (GBL3) and In-Place OTA DFU is only compatible
# with Series 2 devices.
#
# Prerequisites
# - Linux or Mac environment.
# - PATH_SCMD environment variables to be set.
# - A BLE application containing the Application OTA DFU component generated
# and built with success.
#
# Usage
# - Run this script.
# - Copy the generated GBL file onto your smartphone.
# - Open the Si Connect (Android or iOS) smartphone app.
# - Scan for the device and connect if found.
# - Select OTA DFU. Browse the GBL file and start the firmware update.
#
# For a more detailed guide see the readme.md file of the
# Bluetooth - SoC Application OTA DFU example.
#
################################################################################

BUILD_AFTIFACT_FORMAT="s37"

################################################################################
# Entry point
################################################################################
if [ -z "$BASH_VERSION" ]; then
    exec bash "$0" "$@"
fi

# Set path for Simplicity Commander
if [[ -n ${PATH_SCMD} ]]; then
    COMMANDER="${PATH_SCMD}/commander"
fi
if [[ ! -f "${COMMANDER}" ]]; then
    echo "Error: Simplicity Commander not found. Please set the PATH_SCMD"
    echo "environment variable according to AN1086 (sections 2.3 and 3.3.2)."
    read -rsp $'Press enter to continue...\n'
    exit
fi

# Locate build artifact
PATH_PROJ="$(dirname "$0")"
PATH_HEX="$(find "${PATH_PROJ}" -type f -iname "*.${BUILD_AFTIFACT_FORMAT}" -exec echo "{}" \;)"
if [[ -z ${PATH_HEX} ]]; then
    echo "Error: No *.${BUILD_AFTIFACT_FORMAT} build artifact found. Was the project built with success?"
    read -rsp $'Press enter to continue...\n'
    exit
fi
echo "Build artifact: ${PATH_HEX}"

# Set output paths
PATH_OUTPUT="$(dirname "${PATH_HEX}")"
PATH_CONFIG="${PATH_OUTPUT}/gbl4_config.yaml"
echo "Config: ${PATH_CONFIG}"
PATH_GBL="${PATH_OUTPUT}/$(basename "${PATH_HEX}" ".${BUILD_AFTIFACT_FORMAT}").gbl"
echo "Output: ${PATH_GBL}"

# Create GBL config
echo "updates:" > "${PATH_CONFIG}"
echo "  - data: $(basename "${PATH_HEX}")" >> "${PATH_CONFIG}"
# Create GBL file
"${COMMANDER}" gbl4 create "${PATH_GBL}" --config "${PATH_CONFIG}" --device sixg301