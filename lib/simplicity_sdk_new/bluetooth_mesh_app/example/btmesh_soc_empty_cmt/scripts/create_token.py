#!/usr/bin/env python3
"""
 * Copyright 2025 Silicon Laboratories Inc. www.silabs.com
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
"""

"""
Create a Bluetooth Mesh token input file for Simplicity Commander from JSON input.

Requires 'cryptography' package for signature parsing:
  pip install cryptography 

JSON input file format example:
{
  "token_version": "0x00000000",
  "uuid": "e4f1c2a3b4d5e6f7890abcde12345678",
  "oob_config": {
    "output_oob_size": "0x08",
    "output_action": "0x001f",
    "input_oob_size": "0x08",
    "input_action": "0x000f"
  }
}

The 'uuid' and 'oob_config' fields are optional.
The 'token_version' field is required and must be a 32-bit hex value with 0x prefix.
If a private ECC PEM key file is provided, the token data will be signed with ECDSA-SHA256.
The output is a hex string representing the token data, including the signature if signed.
The output can be printed to stdout or written to an output file.

OOB configuration values are defined in the Bluetooth Mesh Protocol Specification, section 5.4.1.2.
Provisioning Capabilities, Table 5.24 - 5.27

Simplicity Commander supports reading and writing custom tokens with the --tokendefs option:
  commander tokens read --tokendefs <token_definitions.json>, or
  commander tokens read --tokendefs <token_definitions.json> --outfile <output_file>

  commander tokens write --tokendefs <token_definitions.json>  --tokenfile <token_data_file>

The token definitions JSON file should define the token label and size, for example:
[
  {
    "name": "MESH_TOKEN",
    "page": "LOCKBITS",
    "offset": "0x0480",
    "sizeB": 256,
    "description": "A 256-byte secure static token."
  }
]
The method is only necessary if the token is not defined in the default Simplicity Commander token definitions.

The token data file has the following line format:

TOKEN_LABEL: <hex string>

Where TOKEN_LABEL is defined in the token definitions JSON file, and the hex string is the full token data including signature.
If no hex string is provided, the token is erased (filled with 0xFF bytes).

Example:
MESH_TOKEN: 04010100000010d7e4f1c2a3b4d5e6f7890abcde1234567806d8081f00080f001dff8885cbd903597963ae637e569a4f5a803dff71b43479f41597ab5b2d4900FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFa19a91415265219e62007e8b2f3cdde90dfa76a8bbf803a44000784c6123a7d40ad21c8664d30b27818aa343364692dc39d5af9c904d3a732f605408aa391ace

Where the Length - Tag - Value (LTV) triplets are:
         04                   01                              01000000
Token version length   token version tag      token version (0x00000001 in little endian)

         10                   d7                    e4f1c2a3b4d5e6f7890abcde12345678
    UUID length            UUID tag                              UUID

         06                   d8                 08            1f00             08            0f00
  OOB config length     OOB config tag    Output OOB size, Output action, Input OOB size, Input action

         1d                   ff           8885cbd903597963ae637e569a4f5a803dff71b43479f41597ab5b2d49
    Padding length       Padding tag                      random padding bytes                                     

         00
  Terminator length

FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF....
      Empty flash space

a19a91415265219e62007e8b2f3cdde90dfa76a8bbf803a44000784c6123a7d40ad21c8664d30b27818aa343364692dc39d5af9c904d3a732f605408aa391ace
                           ECDSA-SHA256 signature (r and s components, 64 bytes)


"""

import argparse
import json
import sys
import os
from cryptography.hazmat.primitives.asymmetric.utils import decode_dss_signature
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec

MESH_TOKEN_NAME = "MESH_TOKEN"

TOKEN_VERSION_LABEL = "token_version"
UUID_LABEL = "uuid"
OOB_CONFIG_LABEL = "oob_config"

def read_json_file(filename):
    """Read and parse a JSON file.
    
    Args:
        filename: Path to the JSON file to read
        
    Returns:
        dict: Parsed JSON data
        
    Raises:
        SystemExit: If file not found or JSON parsing fails
    """
    try:
        with open(filename, 'r') as f:
            data = json.load(f)
        return data
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
        sys.exit(1)
    except json.JSONDecodeError as e:
        print(f"Error: Failed to decode JSON - {e}")
        sys.exit(1)

def validate_json(data):
    """Validate the structure and values of JSON input data.
    
    Args:
        data: Dictionary containing parsed JSON data
        
    Raises:
        SystemExit: If validation fails
    """
    required_fields = [
        TOKEN_VERSION_LABEL
    ]

    oob_config_fields = [
        "output_oob_size",
        "output_action", 
        "input_oob_size",
        "input_action"
    ]

    for field in required_fields:
        if field not in data:
            print(f"Error: Missing required field '{field}' in JSON.")
            sys.exit(1)

    # Check that token_version is a 32-bit integer in hex with 0x prefix
    token_version = data.get(TOKEN_VERSION_LABEL, "")
    if not (isinstance(token_version, str) and token_version.startswith("0x")):
        print("Error: 'token_version' must be a hexadecimal string with '0x' prefix.")
        sys.exit(1)
    try:
        value = int(token_version, 16)
        if not (0 <= value <= 0xFFFFFFFF):
            print("Error: 'token_version' must be a 32-bit hexadecimal value.")
            sys.exit(1)
    except ValueError:
        print("Error: 'token_version' is not a valid hexadecimal number.")
        sys.exit(1)

    # Check if uuid field is present and validate only if present
    if "uuid" in data:
        uuid = data["uuid"]
        if not (isinstance(uuid, str) and len(uuid) == 32):
            print("Error: 'uuid' must be a 32-character hexadecimal string representing 16 bytes.")
            sys.exit(1)
        try:
            int(uuid, 16)
        except ValueError:
            print("Error: 'uuid' is not a valid hexadecimal string.")
            sys.exit(1)
  
    if OOB_CONFIG_LABEL in data:
        oob_config = data[OOB_CONFIG_LABEL]

        for field in oob_config_fields:
            if field not in oob_config:
                print(f"Error: Missing required field '{field}' in oob_config.")
                sys.exit(1)

        # Check that output_oob_size is a valid byte in hex with 0x prefix and max value 0x08
        output_oob_size = oob_config.get("output_oob_size", "")
        if not (isinstance(output_oob_size, str) and output_oob_size.startswith("0x")):
            print("Error: 'output_oob_size' must be a hexadecimal string with '0x' prefix.")
            sys.exit(1)
        try:
            value = int(output_oob_size, 16)
            if not (0 <= value <= 0x08):
                print("Error: 'output_oob_size' must be between 0x00 and 0x08. Values greater than 0x08 are RFU.")
                sys.exit(1)
        except ValueError:
            print("Error: 'output_oob_size' is not a valid hexadecimal byte.")
            sys.exit(1)

        # Check that input_oob_size is a valid byte in hex with 0x prefix and max value 0x08
        input_oob_size = oob_config.get("input_oob_size", "")
        if not (isinstance(input_oob_size, str) and input_oob_size.startswith("0x")):
            print("Error: 'input_oob_size' must be a hexadecimal string with '0x' prefix.")
            sys.exit(1)
        try:
            value = int(input_oob_size, 16)
            if not (0 <= value <= 0x08):
                print("Error: 'input_oob_size' must be between 0x00 and 0x08. Values greater than 0x08 are RFU.")
                sys.exit(1)
        except ValueError:
            print("Error: 'input_oob_size' is not a valid hexadecimal byte.")
            sys.exit(1)

        # Check that output_action is a valid 16-bit hex with 0x prefix, only lowest 5 bits used
        output_action = oob_config.get("output_action", "")
        if not (isinstance(output_action, str) and output_action.startswith("0x")):
            print("Error: 'output_action' must be a hexadecimal string with '0x' prefix.")
            sys.exit(1)
        try:
            value = int(output_action, 16)
            if not (0 <= value <= 0xFFFF):
                print("Error: 'output_action' must be a 16-bit hexadecimal value.")
                sys.exit(1)
            if value > 0x1F:
                print("Error: Only the lowest 5 bits of 'output_action' are used as optional bits. Values greater than 0x1F are RFU.")
                sys.exit(1)
        except ValueError:
            print("Error: 'output_action' is not a valid hexadecimal number.")
            sys.exit(1)

        # Check that input_action is a valid 16-bit hex with 0x prefix, only lowest 4 bits used
        input_action = oob_config.get("input_action", "")
        if not (isinstance(input_action, str) and input_action.startswith("0x")):
            print("Error: 'input_action' must be a hexadecimal string with '0x' prefix.")
            sys.exit(1)
        try:
            value = int(input_action, 16)
            if not (0 <= value <= 0xFFFF):
                print("Error: 'input_action' must be a 16-bit hexadecimal value.")
                sys.exit(1)
            if value > 0x0F:
                print("Error: Only the lowest 4 bits of 'input_action' are used as optional bits. Values greater than 0x0F are RFU.")
                sys.exit(1)
        except ValueError:
            print("Error: 'input_action' is not a valid hexadecimal number.")
            sys.exit(1)  

def export_hex_string(data, sign=False):
    """Export token data as a hex string.
    
    Args:
        data: Dictionary containing token data
        sign: Whether to leave space for signature (default: False)
        
    Returns:
        str: Hex string representation of the token data
    """
    # Define tag constants
    TAG_TOKEN_VERSION = "01"
    LEN_TOKEN_VERSION = "04"
    TAG_DEVICE_UUID = "d7"
    LEN_DEVICE_UUID = "10"
    TAG_DEVICE_OOB_CAPS = "d8"
    LEN_DEVICE_OOB_CAPS = "06"
    TAG_PADDING = "ff"
    LEN_PADDING = "1d"
    LEN_TERMINATOR = "00"

    def lsb_hex(hex_str, byte_len):
        """Convert hex string to little-endian byte order."""
        b = bytes.fromhex(hex_str.zfill(byte_len * 2))
        return b[::-1].hex()
  
    hex_str = (
        LEN_TOKEN_VERSION +
        TAG_TOKEN_VERSION + lsb_hex(data["token_version"][2:], 4)
    )

    # Append UUID field only if present
    if "uuid" in data:
        hex_str += LEN_DEVICE_UUID + TAG_DEVICE_UUID + data["uuid"]

    if OOB_CONFIG_LABEL in data:
        oob_config = data[OOB_CONFIG_LABEL]
        hex_str += (
            LEN_DEVICE_OOB_CAPS +
            TAG_DEVICE_OOB_CAPS +
            lsb_hex(oob_config["output_oob_size"][2:], 1) +
            lsb_hex(oob_config["output_action"][2:], 2) +
            lsb_hex(oob_config["input_oob_size"][2:], 1) +
            lsb_hex(oob_config["input_action"][2:], 2)
        )

    # Add some padding with random bytes
    random_padding = os.urandom(64 - 3 - len(hex_str)//2).hex()
    
    hex_str += (
        LEN_PADDING +
        TAG_PADDING + random_padding +
        LEN_TERMINATOR
    )

    # Pad hex_str to 192 bytes (256-64) with 'FF'
    pad_len = (192 * 2) - len(hex_str)
    if pad_len > 0:
        hex_str += 'F' * pad_len

    if not sign:
        # Add 64 bytes (128 hex chars) of 'FF' as signature placeholder
        hex_str += 'F' * 128
  
    return hex_str

def parse_args():
    """Parse command-line arguments.
    
    Returns:
        argparse.Namespace: Parsed command-line arguments
    """
    parser = argparse.ArgumentParser(description="Create mesh token from JSON input.")
    parser.add_argument("input_file", help="Input JSON file")
    parser.add_argument("private_key", nargs="?", help="Optional ECC PEM private key file to sign the token")  
    parser.add_argument("output_file", nargs="?", help="Optional output file for hex string")
    return parser.parse_args()

def main():
    """Main function to create and optionally sign mesh tokens."""
    args = parse_args()
    data = read_json_file(args.input_file)
    validate_json(data)
    hex_string = export_hex_string(data, sign=args.private_key is not None)

    # Sign the token data if private key is provided
    if args.private_key:
        try:
            # Load the private key from PEM file
            with open(args.private_key, 'rb') as key_file:
                private_key = serialization.load_pem_private_key(
                    key_file.read(),
                    password=None
                )
            
            # Convert hex string to bytes for signing
            token_data = bytes.fromhex(hex_string)
            
            # Sign the token data with ECDSA-SHA256
            signature = private_key.sign(
                token_data,
                ec.ECDSA(hashes.SHA256())
            )
            
            # Parse the ASN.1 DER signature to extract r and s components
            r, s = decode_dss_signature(signature)
            r_hex = f"{r:064x}"
            s_hex = f"{s:064x}"
            
            # Append the signature to the hex string
            hex_string = hex_string + r_hex + s_hex

        except FileNotFoundError:
            print(f"Error: Private key file '{args.private_key}' not found.")
            sys.exit(1)
        except Exception as e:
            print(f"Error: Signing failed - {e}")
            sys.exit(1)
    else:
        # Add 64 bytes (128 hex chars) of 'FF' as signature placeholder
        hex_string += 'F' * 128

    if args.output_file:
        with open(args.output_file, "wb") as f:
            f.write((f"{MESH_TOKEN_NAME}: " + hex_string).encode("utf-8"))
    else:
        print(f"{MESH_TOKEN_NAME}: {hex_string}")

if __name__ == "__main__":
  main()
