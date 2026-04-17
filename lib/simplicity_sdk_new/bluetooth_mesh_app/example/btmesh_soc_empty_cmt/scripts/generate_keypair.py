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
Generate an ECC keypair and export the public key in DER format using Python cryptography library.
The public key bytes are also printed in a C string format for easy inclusion in C code.
"""

import os
import argparse
from pathlib import Path
from typing import Optional
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization

def generate_ecc_keys(filename_prefix: str = "ecc") -> None:
    """Generate an ECC keypair and export keys to PEM and DER formats.
    
    Args:
        filename_prefix (str): Prefix for output filenames (default: "ecc")
        
    Returns:
        None
        
    Raises:
        Exception: If key generation or file operations fail
        
    Example:
        >>> generate_ecc_keys("my_key")
        # Creates: keys/my_key_private.pem, keys/my_key_public.der
    """
    # Input validation
    if not filename_prefix or not isinstance(filename_prefix, str):
        raise ValueError("filename_prefix must be a non-empty string")
    
    # Sanitize filename prefix (remove potentially problematic characters)
    import re
    if not re.match(r'^[a-zA-Z0-9_-]+$', filename_prefix):
        raise ValueError("filename_prefix must contain only alphanumeric characters, underscores, and hyphens")
    
    try:
        Path("keys").mkdir(parents=True, exist_ok=True)
        priv_pem = f"{filename_prefix}_private.pem"
        priv_pem_path = f"keys/{priv_pem}"
        pub_der = f"{filename_prefix}_public.der"
        pub_der_path = f"keys/{pub_der}"

        # Generate ECC private key using prime256v1 curve (SECP256R1)
        private_key = ec.generate_private_key(ec.SECP256R1())
        
        # Get the public key
        public_key = private_key.public_key()

        # Serialize private key to PEM format
        private_pem_bytes = private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption()
        )
        
        # Write private key to file
        try:
            with open(priv_pem_path, 'wb') as f:
                f.write(private_pem_bytes)
        except (IOError, OSError) as e:
            raise IOError(f"Failed to write private key file '{priv_pem}': {e}")

        # Serialize public key to DER format
        public_der_bytes = public_key.public_bytes(
            encoding=serialization.Encoding.DER,
            format=serialization.PublicFormat.SubjectPublicKeyInfo
        )
        
        # Write public key DER to file
        try:
            with open(pub_der_path, 'wb') as f:
                f.write(public_der_bytes)
        except (IOError, OSError) as e:
            raise IOError(f"Failed to write public key DER file '{pub_der}': {e}")

        print(f"ECC keypair and DER file generated successfully with prefix '{filename_prefix}'.")

        # Extract the raw public key bytes from DER format
        try:
            # For prime256v1, public key is usually last 65 bytes (0x04 + X + Y)
            # Find the 0x04 marker (uncompressed key) and extract following 64 bytes
            idx = public_der_bytes.find(b'\x04')
            if idx != -1 and len(public_der_bytes) >= idx + 65:
                pubkey_bytes = public_der_bytes[idx:idx+65]
                print(f"Public key bytes:\n {pubkey_bytes.hex()}")
                c_string = ''.join(f'\\x{b:02x}' for b in pubkey_bytes)
                print(f"C string format:\n\"{c_string}\"")
            else:
                print("Could not find public key bytes in DER file.")
        except Exception as e:
            print(f"Error parsing DER bytes: {e}")
      
    except ValueError as e:
        print(f"Input validation error: {e}")
    except IOError as e:
        print(f"File operation error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

def main() -> None:
    """Main function to parse arguments and generate ECC keypair."""
    parser = argparse.ArgumentParser(description="Generate ECC keypair and DER file using Python cryptography library.")
    parser.add_argument("--filename", type=str, default="ecc", help="Prefix for output files")
    args = parser.parse_args()
    generate_ecc_keys(args.filename)

if __name__ == "__main__":
    main()
