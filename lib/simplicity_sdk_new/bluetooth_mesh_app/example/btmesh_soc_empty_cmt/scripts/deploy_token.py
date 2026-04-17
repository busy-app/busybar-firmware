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
"""

"""
Bluetooth Mesh Manufacturing Token Deployment Script

This script automates the complete process of:
1. Generating ECC keypairs (optional)
2. Creating manufacturing tokens with signatures
3. Deploying tokens to devices using Simplicity Commander

The script provides a streamlined workflow for manufacturing token operations.
"""

import argparse
import subprocess
import sys
import os
import json
from typing import Optional


def is_json_file(filepath: str) -> bool:
    """
    Detect if a file is JSON by trying to parse it.
    
    Args:
        filepath: Path to the file to check
        
    Returns:
        bool: True if file is valid JSON, False otherwise
    """
    try:
        with open(filepath, 'r') as f:
            json.load(f)
        return True
    except (json.JSONDecodeError, UnicodeDecodeError):
        return False


def run_command(cmd: list, description: str, check: bool = True) -> subprocess.CompletedProcess:
    """
    Run a command and handle errors.
    
    Args:
        cmd: Command and arguments as a list
        description: Human-readable description of the command
        check: Whether to check return code and exit on failure
        
    Returns:
        CompletedProcess: Result of the command execution
    """
    print(f"Running: {description}")
    print(f"Command: {' '.join(cmd)}")
    
    try:
        result = subprocess.run(cmd, check=check, capture_output=True, text=True)
        if result.stdout:
            print("Output:", result.stdout)
        if result.stderr and result.returncode != 0:
            print("Error:", result.stderr)
        return result
    except subprocess.CalledProcessError as e:
        print(f"Error running {description}: {e}")
        if e.stdout:
            print("stdout:", e.stdout)
        if e.stderr:
            print("stderr:", e.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print(f"Error: Command not found for {description}")
        print(f"Make sure the required tools are installed and in PATH")
        sys.exit(1)


def generate_keypair(filename_prefix: str) -> tuple[str, str]:
    """
    Generate ECC keypair using generate_keypair.py
    
    Args:
        filename_prefix: Prefix for generated key files
        
    Returns:
        tuple: (private_key_path, public_key_path)
    """
    cmd = ["python", "generate_keypair.py", "--filename", filename_prefix]
    run_command(cmd, f"Generating ECC keypair with prefix '{filename_prefix}'")
    
    # Note: generate_keypair.py creates keys in the 'keys/' directory
    private_key_path = f"keys/{filename_prefix}_private.pem"
    public_key_path = f"keys/{filename_prefix}_public.pem"
    
    return private_key_path, public_key_path


def create_token_definition(filename: str = "token_definition.json") -> str:
    """
    Create a token definition file for Simplicity Commander.
    
    Args:
        filename: Name of the token definition file to create
        
    Returns:
        str: Path to the created token definition file
    """
    token_def = [
        {
            "name": "MESH_TOKEN",
            "page": "LOCKBITS",
            "offset": "0x0480",
            "sizeB": 256,
            "description": "A 256-byte secure static token for Bluetooth Mesh."
        }
    ]
    
    with open(filename, 'w') as f:
        json.dump(token_def, f, indent=2)
    
    print(f"Created token definition file: {filename}")
    return filename


def create_token(token_file: str, private_key: Optional[str], output_file: str) -> str:
    """
    Create manufacturing token using create_token.py
    
    Args:
        token_file: Path to JSON input file
        private_key: Path to private key file (optional)
        output_file: Path to output token file
        
    Returns:
        str: Path to the created token file
    """
    cmd = ["python", "create_token.py", token_file]
    
    if private_key:
        cmd.append(private_key)
    else:
        cmd.append("")  # Empty private key argument
    
    cmd.append(output_file)
    
    description = f"Creating token from '{token_file}'"
    if private_key:
        description += f" with private key '{private_key}'"
    
    run_command(cmd, description)
    return output_file


def deploy_token(token_def_file: str, token_data_file: str, device: Optional[str]) -> None:
    """
    Deploy token to device using Simplicity Commander
    
    Args:
        token_def_file: Path to token definition file
        token_data_file: Path to token data file
        device: Device identifier (optional)
    """
    cmd = [
        "commander", "tokens", "write",
        "--tokendefs", token_def_file,
        "--tokenfile", token_data_file
    ]
    
    if device:
        cmd.extend(["--device", device])
    
    description = f"Deploying token to device"
    if device:
        description += f" '{device}'"
    else:
        description += " (auto-detect)"
    
    run_command(cmd, description)


def main():
    """Main function to orchestrate the token deployment process."""
    parser = argparse.ArgumentParser(
        description="Bluetooth Mesh Manufacturing Token Deployment Script",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Generate keypair, create token, and deploy to auto-detected device
  python deploy_token.py token_data.json --generate-keypair

  # Create token with existing key and deploy to specific device
  python deploy_token.py token_data.json --private-key keys/ecc_private.pem --device 440123456

  # Create token without signing and deploy
  python deploy_token.py token_data.json --device 440123456

  # Deploy existing binary token file (auto-detected)
  python deploy_token.py mesh_token_data --device 440123456

  # Deploy existing binary token with auto-created token definition (auto-detected)
  python deploy_token.py existing_token.bin --create-token-def

  # Just create token without deployment
  python deploy_token.py token_data.json --private-key keys/ecc_private.pem --no-deploy
        """
    )
    
    parser.add_argument("token_file", help="Path to JSON input file with token data OR binary token data file")
    
    # Key generation options
    key_group = parser.add_mutually_exclusive_group()
    key_group.add_argument("--generate-keypair", action="store_true",
                          help="Generate new ECC keypair")
    key_group.add_argument("--private-key", type=str,
                          help="Path to existing private key file")
    
    parser.add_argument("--key-prefix", type=str, default="ecc",
                       help="Prefix for generated keypair files (default: ecc)")
    
    # Token options
    parser.add_argument("--token-output", type=str, default="mesh_token_data",
                       help="Output file for token data (default: mesh_token_data)")
    parser.add_argument("--token-def", type=str, default="token_definition.json",
                       help="Token definition file (default: token_definition.json)")
    parser.add_argument("--create-token-def", action="store_true",
                       help="Create token definition file")
    
    # Deployment options
    parser.add_argument("--device", type=str,
                       help="Device identifier for deployment (optional)")
    parser.add_argument("--no-deploy", action="store_true",
                       help="Skip deployment step")
    
    args = parser.parse_args()
    
    # Validate input file
    if not os.path.exists(args.token_file):
        print(f"Error: Input file '{args.token_file}' not found")
        sys.exit(1)
    
    # Smart input detection: determine if input is JSON or binary token
    is_json_input = is_json_file(args.token_file)
    
    private_key_file = None
    token_file = None
    
    # Handle binary token input (skip token creation)
    if not is_json_input:
        print("=== Using Existing Binary Token ===")
        token_file = args.token_file
        print(f"Using binary token file: {token_file}")
        
        # Validate key options for binary mode
        if args.generate_keypair or args.private_key:
            print("Warning: Key options are ignored when using binary token data")
        
        # Auto-create token definition if needed
        if not os.path.exists(args.token_def):
            args.create_token_def = True
    else:
        # Step 1: Handle keypair generation or use existing key
        if args.generate_keypair:
            print("=== Generating ECC Keypair ===")
            private_key_file, public_key_file = generate_keypair(args.key_prefix)
            print(f"Generated keypair: {private_key_file}, {public_key_file}")
            
            # Auto-create token definition when generating keypair
            if not os.path.exists(args.token_def):
                args.create_token_def = True
        elif args.private_key:
            if not os.path.exists(args.private_key):
                print(f"Error: Private key file '{args.private_key}' not found")
                sys.exit(1)
            private_key_file = args.private_key
            print(f"Using existing private key: {private_key_file}")
        else:
            print("No private key specified - creating unsigned token")
        
        # Step 2: Create token definition if requested or needed
        if args.create_token_def:
            print("\n=== Creating Token Definition ===")
            create_token_definition(args.token_def)
        
        # Step 3: Create manufacturing token
        print("\n=== Creating Manufacturing Token ===")
        token_file = create_token(args.token_file, private_key_file, args.token_output)
        print(f"Created token file: {token_file}")
    
    # Create token definition if requested for binary mode
    if not is_json_input and args.create_token_def:
        print("\n=== Creating Token Definition ===")
        create_token_definition(args.token_def)
    
    # Final step: Deploy token (if not skipped)
    if not args.no_deploy:
        print(f"\n=== Deploying Token to Device ===")
        
        # Check if token definition file exists
        if not os.path.exists(args.token_def):
            print(f"Warning: Token definition file '{args.token_def}' not found")
            print("Use --create-token-def to create it, or provide existing file with --token-def")
            sys.exit(1)
        
        deploy_token(args.token_def, token_file, args.device)
        print("Token deployment completed successfully!")
    else:
        print("\nSkipping deployment (--no-deploy specified)")
    
    print("\n=== Token Process Complete ===")
    if not args.no_deploy:
        print("Manufacturing token has been generated and deployed to device.")
    else:
        print(f"Manufacturing token has been generated: {token_file}")
        print(f"Use 'commander tokens write --tokendefs {args.token_def} --tokenfile {token_file}' to deploy manually")


if __name__ == "__main__":
    main()