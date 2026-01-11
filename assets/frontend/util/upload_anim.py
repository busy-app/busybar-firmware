#!/usr/bin/env python3
"""
upload_anim.py

Upload a compiled animation file (.anim) busy bar and start the
`animation_player`, mirroring the upload + launch steps used by `viewer.py`.

Usage:
  python3 assets/frontend/util/upload_anim.py /path/to/local.anim
  python3 assets/frontend/util/upload_anim.py -a 10.0.4.20:23 /path/to/local.anim
  python3 assets/frontend/util/upload_anim.py -d /ext/animations /path/to/local.anim

The script will attempt to import the helper modules under `scripts/flipper` by
inserting the repository `scripts` folder into `sys.path`. This allows running
the script from anywhere without changing the current directory.
"""

from __future__ import annotations

import argparse
import logging
import os
import sys
from pathlib import Path


def find_repo_scripts_path() -> Path:
    # assets/frontend/util/upload_anim.py is located at repository_root/assets/frontend/util
    # so go up three parents to reach repository root, then append 'scripts'
    here = Path(__file__).resolve()
    repo_root = here.parents[3]
    return repo_root / "scripts"


def parse_address(addr: str | None) -> tuple[str, int]:
    if not addr or addr == "auto":
        return ("10.0.4.20", 23)
    if ":" in addr:
        host, port = addr.split(":", 1)
        return (host, int(port))
    return (addr, 23)


def main() -> int:
    parser = argparse.ArgumentParser(description="Upload .anim to device and launch animation_player")
    parser.add_argument("file", help="Local .anim file to upload")
    parser.add_argument("-a", "--address", help="Device address (host or host:port) or 'auto'", default="auto")
    parser.add_argument("-d", "--device-dir", help="Directory on device to place file", default="/ext/animations")
    parser.add_argument("--force", action="store_true", help="Force upload even if file exists (passed to recursive_send)")
    args = parser.parse_args()

    local_path = os.path.normpath(args.file)
    if not os.path.exists(local_path):
        print(f"Local file does not exist: {local_path}")
        return 2

    # Ensure scripts folder is on sys.path so we can import flipper.* modules
    scripts_path = find_repo_scripts_path()
    sys.path.insert(0, str(scripts_path))

    try:
        from flipper.storage_socket import FlipperStorage, FlipperStorageOperations
    except Exception as e:
        print("Failed to import Flipper helper modules. Make sure you run this inside the repository or that 'scripts' exists.")
        raise

    logging.basicConfig(level=logging.INFO)
    logger = logging.getLogger("upload_anim")

    host_port = parse_address(args.address)

    basename = os.path.basename(local_path)
    device_path = os.path.normpath(os.path.join(args.device_dir, basename)).replace(os.sep, "/")

    logger.info(f"Uploading {local_path} -> {device_path} on {host_port}")

    with FlipperStorage(host_port) as storage:
        ops = FlipperStorageOperations(storage)
        ops.recursive_send(device_path, local_path, force=args.force)

        logger.info("Starting player app on device")
        storage.send_and_wait_prompt(f"loader open animation_player {device_path}\r")

    logger.info("Done")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
