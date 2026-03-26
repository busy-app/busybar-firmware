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
import urllib.error
import urllib.parse
import urllib.request


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


def build_signing_service_url(service_url, profile_name):
    parsed = urllib.parse.urlsplit(service_url)
    query = urllib.parse.urlencode({"profile_name": profile_name})
    if parsed.query:
        query = f"{parsed.query}&{query}"
    return urllib.parse.urlunsplit(parsed._replace(query=query))


def _multipart_body(filename, data, boundary):
    header = (
        f"--{boundary}\r\n"
        f'Content-Disposition: form-data; name="file"; filename="{filename}"\r\n'
        "Content-Type: application/octet-stream\r\n\r\n"
    ).encode()
    footer = f"\r\n--{boundary}--\r\n".encode()
    return header + data + footer


def sign_rps_via_service(service_url, token, profile_name, input_rps, output_rps):
    with open(input_rps, "rb") as src:
        input_data = src.read()

    boundary = "bsb-signing-service-boundary"
    body = _multipart_body(os.path.basename(input_rps), input_data, boundary)
    request = urllib.request.Request(
        build_signing_service_url(service_url, profile_name),
        data=body,
        method="POST",
        headers={
            "Authorization": f"Bearer {token}",
            "Content-Type": f"multipart/form-data; boundary={boundary}",
            "Content-Length": str(len(body)),
        },
    )

    try:
        with urllib.request.urlopen(request) as response:
            signed_data = response.read()
    except urllib.error.HTTPError as exc:
        error_body = exc.read().decode(errors="replace")
        print(
            f"Error signing {input_rps} via signing service: HTTP {exc.code}",
            file=sys.stderr,
        )
        if error_body:
            print(error_body, file=sys.stderr)
        return 1
    except urllib.error.URLError as exc:
        print(
            f"Error connecting to signing service for {input_rps}: {exc}",
            file=sys.stderr,
        )
        return 1

    with open(output_rps, "wb") as dst:
        dst.write(signed_data)
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
