#!/usr/bin/env pwsh

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
# Bluetooth - SoC Interoperability Test GBL generator (Series 2)
#
# This script generates the GBL files needed for the interoperability test.
# These files are used for the OTA-DFU tests.
# The GBL3 (Gecko Bootloader version 3) format is used which is compatible
# with Series 2 devices.
#
# Prerequisites
# - Windows PowerShell 2.0 or higher
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

$App1 = "ota-dfu_non_ack"
$App2 = "ota-dfu_ack"
$DeviceName1 = "IOP_Test_1"
$DeviceName2 = "IOP_Test_2"

################################################################################
# Functions
################################################################################
# Converts ELF file to GBL.
function Elf-To-Gbl {
    Param
    (
        [Parameter(Mandatory=$true, Position=0)]
        [string] $PathElf,
        [Parameter(Mandatory=$true, Position=1)]
        [string] $PathGbl
    )

    # Convert ELF to GBL
    if (Test-Path -Path $PathGbl) {
        Remove-Item $PathGbl  # Remove if exists already
    }

    & $Commander gbl create $PathGbl --app $PathElf
}

# Checks the presence of a string in a binary.
function Search-Str-In-Bin {
    Param
    (
        [Parameter(Mandatory=$true, Position=0)]
        [string] $Str,
        [Parameter(Mandatory=$true, Position=1)]
        [string] $File
    )

    $result = Select-String -Path $File -Pattern $Str -CaseSensitive
    if ($null -eq $result) {
        Write-Error "Error: $Str cannot be found in $File"
        Write-Output "Press any key to continue..."
        [void][Console]::ReadKey()
        Exit(1)
    }
}

################################################################################
# Entry point
################################################################################
if ($PSScriptRoot -eq $Null) {
    # Backward compatibility. $PSScriptRoot is only supported since PowerShell 3.0
    $PSScriptRoot = Split-Path -Parent -Path $MyInvocation.MyCommand.Definition
}

# Set path for Simplicity Commander
if ($Env:PATH_SCMD) {
    $Commander = "$Env:PATH_SCMD\commander.exe"
}
if ($Commander -eq $Null -or -not (Test-Path -Path $Commander)) {
    Write-Output "Error: Simplicity Commander not found. Please set the PATH_SCMD"
    Write-Output "environment variable according to AN1086 (sections 2.3 and 3.3.2)."
    Write-Output "Press any key to continue..."
    [void][Console]::ReadKey()
    Exit(1)
}

$PathOutput = Join-Path $PSScriptRoot 'output_gbl'

New-Item -ItemType Directory -Path $PathOutput -Force | Out-Null
Write-Output "Output directory: $PathOutput"

Write-Output "**********************************************************************"
Write-Output "Generating gbl file for the default application."
Write-Output "**********************************************************************"
Write-Output ""

# Locate the ELF file
$PathElf1 = (Get-ChildItem -Path $PSScriptRoot -Include ('*.axf', '*.out') -Recurse | ForEach-Object { $_.FullName })
if ($null -eq $PathElf1) {
    Write-Output "Error: Neither *.axf nor *.out file was found. Was the project built with success?"
    Write-Output "Press any key to continue..."
    [void][Console]::ReadKey()
    Exit(1)
}
if ($PathElf1 -is [array]) {
    $PathElf1 = $PathElf1[0] # In case multiple ELF file was found, choose the first one.
    Write-Output "Multiple build artifacts found. Choosing: $PathElf1"
}

$PathGbl1 = "$PathOutput\$App1.gbl"
Elf-To-Gbl -PathElf $PathElf1 -PathGbl $PathGbl1
Search-Str-In-Bin -Str $DeviceName1 -File (Join-Path $PathOutput "$App1.gbl")  # Check device name

Write-Output "**********************************************************************"
Write-Output "Generating gbl file for the modified application."
Write-Output "**********************************************************************"
Write-Output ""

# Make a copy of the ELF file
$PathElf2 = Join-Path $PathOutput (Split-Path -Path $PathElf1 -Leaf)
if (Test-Path -Path $PathElf2) {
    Remove-Item $PathElf2  # Remove if exists already
}
Copy-Item -Path $PathElf1 -Destination $PathElf2

# Change the device name by manipulating the ELF file
if ($PSVersionTable.PSVersion.Major -ge 6) {
    $ByteArg = @{ AsByteStream = $true }
} else {
    $ByteArg = @{ Encoding = 'Byte' }
}

$Data = [BitConverter]::ToString((Get-Content $PathElf2 -ReadCount 0 @ByteArg))
$Data = $Data.Replace([BitConverter]::ToString([Text.Encoding]::UTF8.GetBytes($DeviceName1)),
                      [BitConverter]::ToString([Text.Encoding]::UTF8.GetBytes($DeviceName2)))
$Data = [byte[]] ($Data -split '-' -replace '^', '0x')  # Convert the modified "hex string" back to a byte[] array.
Set-Content $PathElf2 @ByteArg -Value $Data

$PathGbl2 = "$PathOutput\$App2.gbl"
Elf-To-Gbl -PathElf $PathElf2 -PathGbl $PathGbl2
Remove-Item $PathElf2  # Remove the modified ELF file since it is not needed anymore
Search-Str-In-Bin -Str $DeviceName2 -File (Join-Path $PathOutput "$App2.gbl")  # Check device name
Exit(0)
