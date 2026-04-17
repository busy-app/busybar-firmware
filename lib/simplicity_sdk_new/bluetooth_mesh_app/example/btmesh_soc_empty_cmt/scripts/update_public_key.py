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
 *
 * Bluetooth Mesh Manufacturing Token Deployment Script
"""

"""
Update SL_BTMESH_CMT_TOKEN_PUBLIC_KEY in btmesh_soc_empty_cmt.slcp file with public key from .der file.

This script reads a public key from a .der file, extracts the raw 65-byte public key
(0x04 + 32-byte X coordinate + 32-byte Y coordinate), and updates the configuration
in the btmesh_soc_empty_cmt.slcp file in the same directory.
"""

import argparse
import sys
import os
from typing import Optional


def read_public_key_from_der(der_file_path: str) -> Optional[bytes]:
    """
    Read public key from DER file and extract the raw 65-byte public key.
    
    Args:
        der_file_path: Path to the .der file containing the public key
        
    Returns:
        bytes: 65-byte raw public key (0x04 + X + Y coordinates) or None if not found
    """
    try:
        with open(der_file_path, 'rb') as f:
            der_data = f.read()
        
        # Find the 0x04 marker (uncompressed public key format)
        # For SECP256R1, the public key is 65 bytes: 0x04 + 32-byte X + 32-byte Y
        idx = der_data.find(b'\x04')
        if idx != -1 and len(der_data) >= idx + 65:
            public_key_bytes = der_data[idx:idx + 65]
            return public_key_bytes
        else:
            print(f"Error: Could not find 65-byte public key in DER file '{der_file_path}'")
            return None
            
    except FileNotFoundError:
        print(f"Error: DER file '{der_file_path}' not found")
        return None
    except Exception as e:
        print(f"Error reading DER file: {e}")
        return None


def format_public_key_as_array(public_key_bytes: bytes) -> str:
    """
    Format public key bytes as C-style array string.
    
    Args:
        public_key_bytes: 65-byte public key
        
    Returns:
        str: Formatted array string like "{0x04,0xc4,0xa0,...}"
    """
    hex_values = [f"0x{b:02x}" for b in public_key_bytes]
    return "{" + ",".join(hex_values) + "}"


def update_slcp_file(slcp_file_path: str, new_public_key_value: str) -> bool:
    """
    Update the SL_BTMESH_CMT_TOKEN_PUBLIC_KEY value in the .slcp file.
    
    Args:
        slcp_file_path: Path to the .slcp file
        new_public_key_value: New public key value as formatted array string
        
    Returns:
        bool: True if successful, False otherwise
    """
    try:
        with open(slcp_file_path, 'r') as f:
            lines = f.readlines()
        
        # Find and update the SL_BTMESH_CMT_TOKEN_PUBLIC_KEY configuration
        updated = False
        for i, line in enumerate(lines):
            if 'SL_BTMESH_CMT_TOKEN_PUBLIC_KEY' in line and line.strip().startswith('- name:'):
                # Found the configuration name, next line should contain the value
                if i + 1 < len(lines) and 'value:' in lines[i + 1]:
                    # Update the value line with quotes around the array
                    indent = lines[i + 1][:lines[i + 1].index('value:')]
                    lines[i + 1] = f"{indent}value: \"{new_public_key_value}\"\n"
                    updated = True
                    break
        
        if not updated:
            print(f"Error: SL_BTMESH_CMT_TOKEN_PUBLIC_KEY configuration not found in '{slcp_file_path}'")
            return False
        
        # Write the updated content back to the file
        with open(slcp_file_path, 'w') as f:
            f.writelines(lines)
        
        print(f"Successfully updated SL_BTMESH_CMT_TOKEN_PUBLIC_KEY in '{slcp_file_path}'")
        return True
        
    except FileNotFoundError:
        print(f"Error: SLCP file '{slcp_file_path}' not found")
        return False
    except Exception as e:
        print(f"Error updating SLCP file: {e}")
        return False


def main():
    """Main function to parse arguments and update the public key."""
    parser = argparse.ArgumentParser(
        description="Update SL_BTMESH_CMT_TOKEN_PUBLIC_KEY in btmesh_soc_empty_cmt.slcp with public key from .der file"
    )
    parser.add_argument("der_file", help="Path to the .der file containing the public key")
    
    args = parser.parse_args()
    
    # Get the directory where this script is located
    script_dir = os.path.dirname(os.path.abspath(__file__))
    # The .slcp file is in the parent directory of the scripts folder
    parent_dir = os.path.dirname(script_dir)
    slcp_file_path = os.path.join(parent_dir, "btmesh_soc_empty_cmt.slcp")
    
    # Validate input files
    if not os.path.exists(args.der_file):
        print(f"Error: DER file '{args.der_file}' does not exist")
        sys.exit(1)
    
    if not os.path.exists(slcp_file_path):
        print(f"Error: SLCP file '{slcp_file_path}' does not exist")
        sys.exit(1)
    
    # Read public key from DER file
    public_key_bytes = read_public_key_from_der(args.der_file)
    if public_key_bytes is None:
        sys.exit(1)
    
    # Format as array string
    public_key_array = format_public_key_as_array(public_key_bytes)
    
    print(f"Extracted public key ({len(public_key_bytes)} bytes):")
    print(f"  Hex: {public_key_bytes.hex()}")
    print(f"  Array format: {public_key_array}")
    
    # Update SLCP file
    if update_slcp_file(slcp_file_path, public_key_array):
        print("Update completed successfully!")
    else:
        sys.exit(1)


if __name__ == "__main__":
    main()