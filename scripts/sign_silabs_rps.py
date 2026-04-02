#!/usr/bin/env python3

"""
Sign Silicon Labs RPS firmware files using commander-cli.

Usage:
    python3 sign_silabs_rps.py --keystore keys.json --input firmware.rps --output firmware_signed.rps
    python3 sign_silabs_rps.py --keystore keys.json --input firmware.rps --output firmware_signed.rps --commander /path/to/commander-cli
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
from http.client import HTTPResponse, HTTPSConnection, HTTPConnection
from urllib.parse import urlsplit, urlencode


def find_commander_cli(user_path=None):
    """Find commander-cli executable."""
    if user_path:
        if os.path.isfile(user_path) and os.access(user_path, os.X_OK):
            return user_path
        raise FileNotFoundError(f"commander-cli not found at: {user_path}")

    # Check PATH for both Linux and macOS binary names
    for name in ("commander-cli", "commander"):
        path = shutil.which(name)
        if path:
            return path

    # macOS default location
    macos_path = "/Applications/Commander-cli.app/Contents/MacOS/commander-cli"
    if os.path.isfile(macos_path):
        return macos_path

    raise FileNotFoundError(
        "commander-cli not found. Install it or pass --commander /path/to/commander-cli"
    )


def detect_image_type(commander, rps_path):
    """Detect whether an RPS file is an M4 or NWP image using commander-cli."""
    result = subprocess.run(
        [commander, "util", "rpsinfo", "--json", rps_path],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(f"Failed to get RPS info for {rps_path}: {result.stderr}")

    info = json.loads(result.stdout)
    image_type = (
        info.get("result", {})
        .get("rps_application_image", {})
        .get("rps_app_1", {})
        .get("image_type", "")
    )

    if "NWP" in image_type:
        return "nwp"
    return "m4"


def sign_rps(commander, keystore, input_rps, output_rps):
    """Sign an RPS file using commander-cli, auto-detecting image type."""
    image_type = detect_image_type(commander, input_rps)
    app_flag = "--nwpapp" if image_type == "nwp" else "--app"

    cmd = [
        commander,
        "rps",
        "convert",
        output_rps,
        "--mic",
        keystore,
        "--sign",
        keystore,
        app_flag,
        input_rps,
    ]

    result = subprocess.run(cmd, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"Error signing {input_rps}:", file=sys.stderr)
        print(result.stderr, file=sys.stderr)
        if result.stdout:
            print(result.stdout, file=sys.stderr)
        return result.returncode

    print(result.stdout, end="")
    return 0


def _build_query(service_url, profile_name):
    """Append profile_name query param to the service URL, return (conn_cls, host, path)."""
    parsed = urlsplit(service_url)
    params = urlencode({"profile_name": profile_name})
    query = f"{parsed.query}&{params}" if parsed.query else params
    path = f"{parsed.path}?{query}"
    conn_cls = HTTPSConnection if parsed.scheme == "https" else HTTPConnection
    host = parsed.netloc
    return conn_cls, host, path


def _encode_multipart(filename, data):
    """Build a multipart/form-data body with a single file field."""
    boundary = "----bsb-firmware-signer-boundary"
    parts = [
        f"--{boundary}\r\n"
        f'Content-Disposition: form-data; name="file"; filename="{filename}"\r\n'
        f"Content-Type: application/octet-stream\r\n\r\n",
    ]
    body = parts[0].encode() + data + f"\r\n--{boundary}--\r\n".encode()
    content_type = f"multipart/form-data; boundary={boundary}"
    return body, content_type


def _read_response_body(resp: HTTPResponse) -> bytes:
    return resp.read()


def sign_rps_via_service(service_url, token, profile_name, input_rps, output_rps):
    with open(input_rps, "rb") as f:
        file_data = f.read()

    conn_cls, host, path = _build_query(service_url, profile_name)
    body, content_type = _encode_multipart(os.path.basename(input_rps), file_data)

    conn = conn_cls(host)
    try:
        conn.request(
            "POST",
            path,
            body=body,
            headers={
                "Authorization": f"Bearer {token}",
                "Content-Type": content_type,
                "Accept": "application/octet-stream",
                "User-Agent": "fbt-firmware-signer/1.0",
            },
        )
        resp = conn.getresponse()
        resp_data = _read_response_body(resp)

        if resp.status != 200:
            print(
                f"Error signing {input_rps} via signing service: HTTP {resp.status}",
                file=sys.stderr,
            )
            if resp_data:
                print(resp_data.decode(errors="replace"), file=sys.stderr)
            return 1
    except OSError as exc:
        print(
            f"Error connecting to signing service for {input_rps}: {exc}",
            file=sys.stderr,
        )
        return 1
    finally:
        conn.close()

    with open(output_rps, "wb") as f:
        f.write(resp_data)
    return 0


def resolve_signing_mode(args):
    service_values = [args.service_url, args.service_token, args.profile]
    if any(service_values):
        if not all(service_values):
            raise ValueError(
                "service signing requires --service-url, --token, and --profile"
            )
        return "service"
    if args.keystore:
        return "local"
    raise ValueError(
        "either --keystore or the service signing options must be provided"
    )


def main():
    parser = argparse.ArgumentParser(description="Sign Silicon Labs RPS firmware files")
    parser.add_argument(
        "--keystore",
        help="Path to JSON keystore file for signing",
    )
    parser.add_argument(
        "--input",
        required=True,
        help="Input RPS file to sign",
    )
    parser.add_argument(
        "--output",
        required=True,
        help="Output path for signed RPS file",
    )
    parser.add_argument(
        "--commander",
        default=None,
        help="Path to commander-cli executable (auto-detected if not specified)",
    )
    parser.add_argument(
        "--service-url",
        default=None,
        help="Signing service /api/v1/sign endpoint URL",
    )
    parser.add_argument(
        "--token",
        dest="service_token",
        default=None,
        help="Bearer token for the signing service",
    )
    parser.add_argument(
        "--profile",
        default=None,
        help="Signing-service profile name to use",
    )
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(f"Error: input file not found: {args.input}", file=sys.stderr)
        return 1

    try:
        mode = resolve_signing_mode(args)
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1

    if mode == "service":
        return sign_rps_via_service(
            args.service_url,
            args.service_token,
            args.profile,
            args.input,
            args.output,
        )

    if not os.path.isfile(args.keystore):
        print(f"Error: keystore file not found: {args.keystore}", file=sys.stderr)
        return 1

    try:
        commander = find_commander_cli(args.commander)
    except FileNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1

    return sign_rps(commander, args.keystore, args.input, args.output)


if __name__ == "__main__":
    sys.exit(main())
