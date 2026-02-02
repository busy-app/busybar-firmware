#!/usr/bin/env python3
"""Provision test Matter attestation, setup params and device info.

This is a Python rewrite of `provision.sh`.
It performs the following steps (same defaults as the original shell script):
 1. (Optional) Ensure we're at repo root by checking `scripts` directory.
 2. Source (approximate) the toolchain environment (best-effort: executed as a subprocess
    to get PATH modifications) unless skipped.
 3. Wipe crypto storage partition 0.
 4. Provision attestation artifacts (private key, DAC, PAI).
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
import os
import secrets
import shutil
import subprocess
import sys
import tempfile
from contextlib import contextmanager, nullcontext
from pathlib import Path
from typing import Iterable


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


SCRIPT_PATH = Path(__file__).resolve()
SCRIPTS_DIR = SCRIPT_PATH.parent.parent
REPO_ROOT = SCRIPTS_DIR.parent

DEFAULT_VENDOR_ID = "158A"
DEFAULT_PRODUCT_ID = "0001"
DEFAULT_PASSCODE = _gen_random_passcode()
DEFAULT_DISCRIMINATOR = _rand_12bit_str()
DEFAULT_CERTS_DIR = SCRIPTS_DIR / "test_certs" / "matter"

CRYPTO_STORAGE = SCRIPTS_DIR / "crypto_storage.py"
CREDENTIALS = SCRIPTS_DIR / "credentials.py"


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


@contextmanager
def production_certs_bundle(production: Path):
    temp = Path(tempfile.mkdtemp(prefix="bsb-matter-certs"))
    try:
        src_pai_and_paa = production / "certificate_chain.pem"
        src_dac_cert = production / "certificate.pem"
        src_dac_key = production / "privateKey.pem"
        pai_cert = temp / "pai.pem"
        paa_cert = temp / "paa.pem"

        ensure_files_exist([src_pai_and_paa, src_dac_cert, src_dac_key])

        delimiter = "-----BEGIN CERTIFICATE-----"
        with open(src_pai_and_paa, "r", encoding="utf-8") as source:
            pai_and_paa = source.read()
        certs = [delimiter + cert for cert in pai_and_paa.split(delimiter) if cert]
        if len(certs) < 2:
            raise RuntimeError(
                "certificate_chain.pem must contain both PAI and PAA certificates"
            )
        pai, paa = certs[:2]
        pai_cert.write_text(pai, encoding="utf-8")
        paa_cert.write_text(paa, encoding="utf-8")

        yield pai_cert, src_dac_key, src_dac_cert
    finally:
        shutil.rmtree(temp, ignore_errors=True)


def ensure_files_exist(paths: Iterable[Path]):
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


def parse_args(argv):
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
        type=Path,
        default=None,
        help="Directory containing production certificates from CloudPKI. If not set, default test certs will be used.",
    )
    parser.add_argument(
        "--toolchain-path",
        type=Path,
        default=REPO_ROOT,
        help="Path to repository root containing the toolchain/ directory (default: repo root inferred from script location)",
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
    return parser.parse_args(argv)


def normalize_numeric(value: str) -> str:
    return str(int(value, 16)) if value.lower().startswith("0x") else value


def wipe_crypto_storage(env: dict) -> None:
    run_cmd(
        [sys.executable, str(CRYPTO_STORAGE), "wipe", "-P", "0"],
        env=env,
        desc="wipe crypto storage",
    )


def provision_attestation(
    env: dict, pai_cert: Path, dac_key: Path, dac_cert: Path
) -> None:
    ensure_files_exist([pai_cert, dac_key, dac_cert])
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
        ],
        env=env,
        desc="provision attestation",
    )


def provision_setup(env: dict, passcode: str, discriminator: str) -> None:
    run_cmd(
        [
            sys.executable,
            str(CREDENTIALS),
            "setup",
            "-d",
            discriminator,
            "-p",
            passcode,
        ],
        env=env,
        desc="provision setup",
    )


def provision_device_info(env: dict) -> None:
    run_cmd(
        [sys.executable, str(CREDENTIALS), "info"],
        env=env,
        desc="provision device info",
    )


def main(argv=None):
    args = parse_args(sys.argv[1:] if argv is None else argv)

    env = os.environ.copy()
    toolchain_path = args.toolchain_path.expanduser().resolve()
    if not args.no_toolchain_env:
        setup_toolchain_env(env, toolchain_path, args.toolchain_version)

    production_dir = (
        args.production_certs.expanduser().resolve()
        if args.production_certs is not None
        else None
    )

    wipe_crypto_storage(env)

    if not args.no_attest:
        if production_dir is not None:
            attestation_context = production_certs_bundle(production_dir)
        else:
            attestation_context = nullcontext(
                get_default_certs(DEFAULT_CERTS_DIR, args.vendor_id, args.product_id)
            )
        with attestation_context as (pai_cert, dac_key, dac_cert):
            provision_attestation(env, pai_cert, dac_key, dac_cert)

    if not args.no_setup:
        provision_setup(
            env,
            normalize_numeric(args.passcode),
            normalize_numeric(args.discriminator),
        )

    if not args.no_info:
        provision_device_info(env)

    print(f"Passcode: {args.passcode}, Discriminator: {args.discriminator}")
    print("Provisioning complete.")


if __name__ == "__main__":
    main()
