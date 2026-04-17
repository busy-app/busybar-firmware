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
# Bluetooth - SoC Interoperability Test GBL generator (Series 3)
#
# This script generates the GBL files needed for the interoperability test.
# These files are used for the OTA-DFU tests.
# The GBL4 (Gecko Bootloader version 4) format is used which is compatible
# with Series 3 devices.
#
# Prerequisites
# - Linux or Mac environment.
# - The presence of the sed (stream editor) package.
# - PATH_SCMD environment variables to be set. See readme.md
# for more information.
# - The Bluetooth - SoC Interoperability Test example generated and built
# with success.
#
# Usage
# - Run this script.
# - Copy the generated gbl files onto your smartphone.
# - In the Simplicity Connect app, open the Interoperability Test demo tile.
# - Start the test. When prompted to choose a gbl file for OTA-DFU, select the
# ota-dfu_ack.gbl file. When prompted again, select ota-dfu_non_ack.gbl.
#
# For a more detailed guide see the readme.md file of the example.
#
################################################################################

APP_1="ota-dfu_non_ack"
APP_2="ota-dfu_ack"
DEVICE_NAME_1="IOP_Test_1"
DEVICE_NAME_2="IOP_Test_2"

################################################################################
# Functions
################################################################################
# Converts ELF file to GBL.
function elf_to_gbl () {
    # Convert ELF to GBL
    if [ -f "${2}" ]; then
        rm "${2}"  # Remove if exists already
    fi

    "${COMMANDER}" gbl4 create "${2}" --data "${1}" --device sixg3
}

# Checks the presence of a string in a binary.
function search_str_in_bin () {
    grep -aq $1 $2
    if [[ $? -ne 0 ]]; then
        echo "Error: $1 cannot be found in $2"
        read -rsp $'Press enter to continue...\n'
        exit
    fi
}

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

PATH_PROJ="$(dirname $0)"
PATH_OUTPUT="${PATH_PROJ}/output_gbl"

mkdir -p ${PATH_OUTPUT}
echo "Output directory: ${PATH_OUTPUT}"

echo "**********************************************************************"
echo "Generating gbl file for the default application."
echo "**********************************************************************"
echo

# Locate ELF file
PATH_ELF=`find ${PATH_PROJ} -type f \( -iname "*.axf" \) -exec echo {} \;`
if [[ -z ${PATH_ELF} ]]; then
    PATH_ELF=`find ${PATH_PROJ} -type f \( -iname "*.out" \) -exec echo {} \;`
fi
if [[ -z ${PATH_ELF} ]]; then
    echo "Error: Neither *.axf nor *.out found. Was the project built with success?"
    read -rsp $'Press enter to continue...\n'
    exit
fi

# Make a copy of the ELF file
PATH_ELF_COPY="${PATH_OUTPUT}/$(basename "${PATH_ELF}")"
if [ -f "${PATH_ELF_COPY}" ]; then
    rm "${PATH_ELF_COPY}"  # Remove if exists already
fi
cp "${PATH_ELF}" "${PATH_ELF_COPY}"

PATH_GBL_1="${PATH_OUTPUT}/${APP_1}.gbl"
elf_to_gbl "${PATH_ELF_COPY}" "${PATH_GBL_1}"
search_str_in_bin ${DEVICE_NAME_1} "${PATH_GBL_1}"  # Check device name

echo "**********************************************************************"
echo "Generating gbl file for the modified application."
echo "**********************************************************************"
echo

# Change the device name by manipulating the ELF file
if [ "$(uname)" = "Darwin" ]; then
  LC_ALL=C sed -i '.backup' -e "s/${DEVICE_NAME_1}/${DEVICE_NAME_2}/" "${PATH_ELF_COPY}"
else
  sed -bi "s/${DEVICE_NAME_1}/${DEVICE_NAME_2}/" "${PATH_ELF_COPY}"
fi
retVal=$?
if [ $retVal -ne 0 ]; then
  echo "Error $retVal: Failed to change the device name in ${PATH_ELF_COPY}."
  read -rsp $'Press enter to continue...\n'
  exit
fi

PATH_GBL_2="${PATH_OUTPUT}/${APP_2}.gbl"
elf_to_gbl "${PATH_ELF_COPY}" "${PATH_GBL_2}"
rm "${PATH_ELF_COPY}"  # Remove the modified ELF file since it is not needed anymore
search_str_in_bin ${DEVICE_NAME_2} "${PATH_GBL_2}" # Check device name
