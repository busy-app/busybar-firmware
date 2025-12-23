#!/usr/bin/env python3
"""Provision test Matter attestation, setup params and device info.

This is a Python rewrite of `provision.sh`.
It performs the following steps (same defaults as the original shell script):
 1. (Optional) Ensure we're at repo root by checking `scripts` directory.
 2. Source (approximate) the toolchain environment (best-effort: executed as a subprocess
    to get PATH modifications) unless skipped.
 3. Wipe crypto storage partition 0.
 4. Provision attestation artifacts (private key, DAC, PAI, CD).
 5. Provision setup parameters (discriminator + passcode; salt/verifier generated internally by credentials.py).
 6. Provision device info (uses defaults provided by credentials.py `info` subcommand).

Exit codes:
 0 success, non-zero on first failure.

Assumptions:
 - `credentials.py` and `crypto_storage.py` are executable and importable from the `scripts` directory.
 - Network/CLI connectivity matches what those scripts expect.

You can override vendor/product IDs, passcode, discriminator, and certs dir via CLI flags.
"""
from __future__ import annotations

import argparse
import subprocess
import sys
import os
import secrets
import tempfile
import shutil
from pathlib import Path


def _disallowed_passcode(s: str) -> bool:
    """Heuristic disallowed passcodes per common Matter guidance.

    This rejects trivial/weak values while keeping the spec's intent that the
    initial randomness is 27 bits before any rejection.

    Rules:
      - All digits identical (e.g., 00000000, 11111111, ...)
      - Common sequential patterns (ascending/descending)
    """
    # All digits same
    if len(set(s)) == 1:
        return True
    # Common sequences
    sequences = {
        "01234567",
        "12345678",
        "23456789",
        "34567890",
        "98765432",
        "87654321",
        "76543210",
    }
    if s in sequences:
        return True
    return False


def _gen_random_passcode() -> str:
    """Generate an 8-digit decimal passcode from a 27-bit random seed.

    The seed is sampled with 27 bits of entropy, then mapped into the 8-digit
    decimal space by modulo reduction; values matching disallowed patterns are
    rejected and re-sampled, preserving the 27-bit initial entropy requirement.
    """
    while True:
        seed = secrets.randbits(27)  # 27 bits of entropy as required
        code = seed % 100_000_000  # map to 8-digit decimal space
        s = f"{code:08d}"
        if _disallowed_passcode(s):
            continue
        return s


def _rand_12bit_str() -> str:
    """Return decimal string of a random 12-bit value (0..4095)."""
    return str(secrets.randbelow(1 << 12))


REPO_ROOT_MARKER = "scripts"
DEFAULT_VENDOR_ID = "158A"
DEFAULT_PRODUCT_ID = "0001"
DEFAULT_PASSCODE = _gen_random_passcode()
DEFAULT_DISCRIMINATOR = _rand_12bit_str()
DEFAULT_CERTS_DIR = "scripts/test_certs/matter"
DEFAULT_CD_PATH = f"{DEFAULT_CERTS_DIR}/test-CD-{DEFAULT_VENDOR_ID}-{DEFAULT_PRODUCT_ID}.der"

# Relative paths used by original script
CRYPTO_STORAGE = Path("scripts/crypto_storage.py")
CREDENTIALS = Path("scripts/credentials.py")

to_cleanup = []


def check_repo_root() -> None:
    if not Path(REPO_ROOT_MARKER).is_dir():
        print(
            "Error: Must run from repository root (missing 'scripts' directory).",
            file=sys.stderr,
        )
        sys.exit(1)


def run_cmd(cmd: list[str], env=None, desc: str = "") -> None:
    try:
        subprocess.run(cmd, check=True, env=env)
    except subprocess.CalledProcessError as e:
        if desc:
            print(f"Failed: {desc}", file=sys.stderr)
        print(
            f"Command failed with exit code {e.returncode}: {' '.join(cmd)}",
            file=sys.stderr,
        )
        sys.exit(e.returncode)


def get_default_certs(certs_dir: Path, vendor_id: str, product_id: str):
    pai_cert = certs_dir / f"test-PAI-{vendor_id}-cert.pem"
    dac_key = certs_dir / f"test-DAC-{vendor_id}-{product_id}-key.pem"
    dac_cert = certs_dir / f"test-DAC-{vendor_id}-{product_id}-cert.pem"
    return pai_cert, dac_key, dac_cert


def get_production_certs(production: Path):
    temp = Path(tempfile.mkdtemp(prefix="bsb-matter-certs"))
    to_cleanup.append(temp)

    src_pai_and_paa = production / "certificate_chain.pem"
    src_dac_cert = production / "certificate.pem"
    src_dac_key = production / "privateKey.pem"
    pai_cert = temp / "pai.pem"
    paa_cert = temp / "paa.pem"

    # CloudPKI provides concatenated PAI and PAA, we need to split them
    with open(src_pai_and_paa, "r") as src_pai_and_paa:
        pai_and_paa = src_pai_and_paa.read()
        DELIMITER = "-----BEGIN CERTIFICATE-----"
        certs = pai_and_paa.split(DELIMITER)
        certs = [DELIMITER + cert for cert in certs if cert]
        pai = certs[0]
        paa = certs[1]
        with open(pai_cert, "w") as dst_pai:
            dst_pai.write(pai)
        with open(paa_cert, "w") as dst_paa:
            dst_paa.write(paa)

    return pai_cert, src_dac_key, src_dac_cert    


def ensure_files_exist(paths: list[Path]):
    missing = [p for p in paths if not p.is_file()]
    if missing:
        for p in missing:
            print(f"Missing required file: {p}", file=sys.stderr)
        sys.exit(2)


def setup_toolchain_env(
    env: dict, toolchain_root: Path, version: str | None = None
) -> None:
    """Mimic minimal fbtenv.sh behavior in pure Python (cross-platform).

    - Detect host OS + arch
    - Construct toolchain directory: <toolchain_root>/toolchain/<arch>-<os>
    - If exists, prepend its /bin to PATH
    - Optionally validate VERSION file if version provided
    - Set SSL_CERT_FILE & REQUESTS_CA_BUNDLE if certifi bundle found
    - Apply a restricted Python environment similar to fbtenv (optional; minimal here)
    """
    import platform
    import glob

    system = platform.system().lower()  # 'darwin', 'linux', 'windows'
    if system.startswith("msys") or system.startswith("cygwin"):
        system = "windows"
    if system not in ("darwin", "linux"):
        # Toolchain likely unsupported; skip silently
        return

    machine = platform.machine().lower()
    if machine in ("x86_64", "amd64"):
        arch = "x86_64"
    elif machine in ("arm64", "aarch64"):
        arch = "aarch64"
    else:
        arch = machine  # fallback

    arch_dir = toolchain_root / "toolchain" / f"{arch}-{system}"
    bin_dir = arch_dir / "bin"
    if not bin_dir.is_dir():
        # Nothing to do if toolchain not present
        return

    # VERSION check
    if version is not None:
        version_file = arch_dir / "VERSION"
        if version_file.is_file():
            try:
                content = version_file.read_text().strip()
                if content != version:
                    print(
                        f"Warning: Toolchain version mismatch (expected {version}, found {content}).",
                        file=sys.stderr,
                    )
            except Exception:
                pass

    # Prepend toolchain bin
    path_parts = env.get("PATH", "").split(os.pathsep)
    if str(bin_dir) not in path_parts:
        env["PATH"] = os.pathsep.join([str(bin_dir)] + path_parts)

    # Locate certifi CA bundle inside toolchain (pattern search)
    cert_glob = glob.glob(
        str(arch_dir / "lib" / "python*" / "site-packages" / "certifi" / "cacert.pem")
    )
    if cert_glob:
        cacert = cert_glob[0]
        env["SSL_CERT_FILE"] = cacert
        env["REQUESTS_CA_BUNDLE"] = cacert

    # Mirror minimal isolation flags
    env["PYTHONNOUSERSITE"] = "1"
    # We intentionally do NOT clear PYTHONPATH/HOME since we run inside user's interpreter
    # but keep the option to clear if needed via flags later.

    # Mark environment applied
    env["FBT_PY_ENV_APPLIED"] = "1"


def main():
    parser = argparse.ArgumentParser(
        description="Provision test certificates and setup parameters (Python version)"
    )
    parser.add_argument(
        "--vendor-id",
        default=DEFAULT_VENDOR_ID,
        help="Vendor ID (hex string as in filenames)",
    )
    parser.add_argument(
        "--product-id",
        default=DEFAULT_PRODUCT_ID,
        help="Product ID (hex string as in filenames)",
    )
    parser.add_argument(
        "--passcode", default=DEFAULT_PASSCODE, help="Setup passcode (decimal string)"
    )
    parser.add_argument(
        "--discriminator",
        default=DEFAULT_DISCRIMINATOR,
        help="Setup discriminator (decimal string)",
    )
    parser.add_argument(
        "--production-certs",
        default=None,
        help="Directory containing production certificates from CloudPKI. If not set, default test certs will be used.",
    )
    parser.add_argument(
        "--cd", default=DEFAULT_CD_PATH, help="Path to CD DER file. If not, default test CD will be used."
    )
    parser.add_argument(
        "--toolchain-path",
        default=".",
        help="Path to repository root containing the toolchain/ directory (default: current dir)",
    )
    parser.add_argument(
        "--toolchain-version",
        default=None,
        help="Expected toolchain version (compared with toolchain/<arch-os>/VERSION).",
    )
    parser.add_argument(
        "--no-toolchain-env",
        action="store_true",
        help="Skip automatic toolchain environment setup (pure host env).",
    )
    parser.add_argument(
        "--no-attest", action="store_true", help="Skip attestation provisioning"
    )
    parser.add_argument(
        "--no-setup", action="store_true", help="Skip setup provisioning"
    )
    parser.add_argument(
        "--no-info", action="store_true", help="Skip device info provisioning"
    )

    args = parser.parse_args()

    check_repo_root()

    production_certs = args.production_certs
    if args.production_certs:
        pai_cert, dac_key, dac_cert = get_production_certs(Path(production_certs))
    else:
        pai_cert, dac_key, dac_cert = get_default_certs(
            Path(DEFAULT_CERTS_DIR), args.vendor_id, args.product_id
        )

    cd_file = Path(args.cd)

    # Setup toolchain environment cross-platform (no shell sourcing)
    env = os.environ.copy()
    if not args.no_toolchain_env:
        setup_toolchain_env(env, Path(args.toolchain_path), args.toolchain_version)

    # Step 1: wipe crypto storage partition 0
    run_cmd(
        [sys.executable, str(CRYPTO_STORAGE), "wipe", "-P", "0"],
        env=env,
        desc="wipe crypto storage",
    )

    # Step 2: Attestation files
    if not args.no_attest:
        ensure_files_exist([pai_cert, dac_key, dac_cert, cd_file])
        run_cmd(
            [
                sys.executable,
                str(CREDENTIALS),
                "attest",
                "--key",
                str(dac_key),
                "--dac",
                str(dac_cert),
                "--pai",
                str(pai_cert),
                "--cd",
                str(cd_file),
            ],
            env=env,
            desc="provision attestation",
        )

    # Step 3: Setup parameters
    if not args.no_setup:
        run_cmd(
            [
                sys.executable,
                str(CREDENTIALS),
                "setup",
                "-d",
                str(
                    int(args.discriminator, 0)
                    if args.discriminator.startswith("0x")
                    else args.discriminator
                ),
                "-p",
                str(
                    int(args.passcode, 0)
                    if args.passcode.startswith("0x")
                    else args.passcode
                ),
            ],
            env=env,
            desc="provision setup",
        )

    # Step 4: Device info
    if not args.no_info:
        run_cmd(
            [sys.executable, str(CREDENTIALS), "info"],
            env=env,
            desc="provision device info",
        )

    print(f"Passcode: {args.passcode}, Discriminator: {args.discriminator}")
    print("Provisioning complete.")

    for path in to_cleanup:
        shutil.rmtree(path)


if __name__ == "__main__":
    main()
