#!/usr/bin/env python3
"""Provision Matter attestation, setup params and device info.

Performs the following steps:
 1. Wipe crypto storage partition 0.
 2. Provision attestation artifacts (private key, DAC, PAI).
 3. Provision setup parameters (SPAKE2+ salt/verifier, discriminator, passcode).
 4. Provision device info (vendor/product IDs, names, serial number, etc.).

Uses a single CryptoStorage connection for all operations.
"""

import hashlib
import secrets
import shutil
import struct
import tempfile
from contextlib import contextmanager
from pathlib import Path
from random import randbytes

from cryptography import x509
from cryptography.hazmat.primitives import serialization

from ecdsa.curves import NIST256p

from flipper.cli import Cli
from flipper.app import App, CatchExceptions
from crypto_storage import CryptoStorage
from credentials import (
    Partition,
    KeyType,
    WriteFlag,
    AttestationKeyId,
    SetupKeyID,
    DeviceInfoKeyID,
    to_terminated,
    pack_current_date,
)

SCRIPT_PATH = Path(__file__).resolve()
SCRIPTS_DIR = SCRIPT_PATH.parent

DEFAULT_VENDOR_ID = "158A"
DEFAULT_PRODUCT_ID = "BB01"
DEFAULT_CERTS_DIR = SCRIPTS_DIR / "test_certs" / "matter"


def _disallowed_passcode(s: str) -> bool:
    if len(set(s)) == 1:
        return True
    return s in {
        "01234567",
        "12345678",
        "23456789",
        "34567890",
        "98765432",
        "87654321",
        "76543210",
    }


def _gen_random_passcode() -> str:
    """Generate an 8-digit decimal passcode from a 27-bit random seed."""
    while True:
        seed = secrets.randbits(27)
        s = f"{seed % 100_000_000:08d}"
        if not _disallowed_passcode(s):
            return s


def _rand_12bit_str() -> str:
    return str(secrets.randbelow(1 << 12))


def _get_device_info(portname) -> dict[str, str]:
    """Run device_info once and return all key-value pairs."""
    with Cli(portname) as cli:
        cli.send("device_info\r")
        raw = cli.read.until(cli.CLI_PROMPT)
    result = {}
    for line in raw.decode("utf-8").splitlines():
        if ": " not in line:
            continue
        key, _, value = line.partition(": ")
        result[key.strip()] = value.strip()
    return result


@contextmanager
def _production_certs_bundle(production: Path):
    """Extract PAI from certificate_chain.pem; yield (pai, dac_key, dac_cert) paths."""
    temp = Path(tempfile.mkdtemp(prefix="bsb-matter-certs"))
    try:
        src_chain = production / "certificate_chain.pem"
        src_dac_cert = production / "certificate.pem"
        src_dac_key = production / "privateKey.pem"

        for p in (src_chain, src_dac_cert, src_dac_key):
            if not p.is_file():
                raise FileNotFoundError(f"Missing required file: {p}")

        delimiter = "-----BEGIN CERTIFICATE-----"
        pai_and_paa = src_chain.read_text(encoding="utf-8")
        certs = [delimiter + cert for cert in pai_and_paa.split(delimiter) if cert]
        if len(certs) < 2:
            raise RuntimeError(
                "certificate_chain.pem must contain both PAI and PAA certificates"
            )

        pai_cert = temp / "pai.pem"
        pai_cert.write_text(certs[0], encoding="utf-8")

        yield pai_cert, src_dac_key, src_dac_cert
    finally:
        shutil.rmtree(temp, ignore_errors=True)


class Main(App):
    def init(self):
        self.parser.add_argument(
            "--vendor-id",
            default=DEFAULT_VENDOR_ID,
            help="Vendor ID (hex string as in filenames)",
        )
        self.parser.add_argument(
            "--product-id",
            default=DEFAULT_PRODUCT_ID,
            help="Product ID (hex string as in filenames)",
        )
        self.parser.add_argument(
            "--passcode",
            default=_gen_random_passcode(),
            help="Setup passcode (decimal string)",
        )
        self.parser.add_argument(
            "--discriminator",
            default=_rand_12bit_str(),
            help="Setup discriminator (decimal string)",
        )
        self.parser.add_argument(
            "--production-certs",
            type=Path,
            default=None,
            help="Directory containing production certificates from CloudPKI",
        )
        self.parser.add_argument(
            "--no-attest",
            action="store_true",
            help="Skip attestation provisioning",
        )
        self.parser.add_argument(
            "--no-setup",
            action="store_true",
            help="Skip setup provisioning",
        )
        self.parser.add_argument(
            "--no-info",
            action="store_true",
            help="Skip device info provisioning",
        )
        self.parser.add_argument(
            "--insecure-crypto",
            action="store_true",
            default=False,
            help="Use plain (unwrapped) key storage; by default keys are wrapped",
        )
        self.parser.set_defaults(func=self.provision)

    def get_portname(self):
        return ("10.0.4.20", 23)

    # -- file I/O helpers -------------------------------------------------

    @staticmethod
    def read_cert_file(path: Path) -> bytes:
        with open(path, "rb") as f:
            data = f.read()
        if path.suffix.lower() == ".pem":
            return x509.load_pem_x509_certificate(data).public_bytes(
                serialization.Encoding.DER
            )
        elif path.suffix.lower() == ".der":
            return data
        raise ValueError(f"Unsupported certificate format: {path.suffix}")

    @staticmethod
    def read_key_file(path: Path) -> bytes:
        with open(path, "rb") as f:
            data = f.read()
        if path.suffix.lower() == ".pem":
            key = serialization.load_pem_private_key(data, None)
        elif path.suffix.lower() == ".der":
            key = serialization.load_der_private_key(data, None)
        else:
            raise ValueError(f"Unsupported key format: {path.suffix}")
        return key.private_numbers().private_value.to_bytes(32, "big")

    @staticmethod
    def generate_spake2_values(
        passcode: int, salt_len: int = 32, iter_count: int = 1000
    ) -> tuple[bytes, bytes]:
        salt = randbytes(salt_len)
        ws_len = NIST256p.baselen + 8
        ws = hashlib.pbkdf2_hmac(
            "sha256", struct.pack("<I", passcode), salt, iter_count, ws_len * 2
        )
        w0 = int.from_bytes(ws[:ws_len], byteorder="big") % NIST256p.order
        w1 = int.from_bytes(ws[ws_len:], byteorder="big") % NIST256p.order
        L = NIST256p.generator * w1
        verifier = w0.to_bytes(NIST256p.baselen, byteorder="big") + L.to_bytes(
            "uncompressed"
        )
        return salt, verifier

    @staticmethod
    def normalize_numeric(value: str) -> int:
        return int(value, 16) if value.lower().startswith("0x") else int(value)

    # -- storage helpers --------------------------------------------------

    @staticmethod
    def write_data(
        storage: CryptoStorage,
        key_type: int,
        data: dict[int, bytes],
        wrap: bool = False,
    ):
        flags = WriteFlag.WRAP if wrap else WriteFlag.NONE
        for key_id, key_value in data.items():
            ret = storage.write_key(
                Partition.MAIN,
                key_type,
                key_id,
                flags,
                len(key_value),
                key_value.hex(),
                echo=False,
            )
            if ret != 0:
                raise Exception(f"write_key failed with error {ret}")

    # -- provisioning steps -----------------------------------------------

    def provision_attestation(
        self,
        storage: CryptoStorage,
        pai_cert: Path,
        dac_key: Path,
        dac_cert: Path,
        wrap: bool,
    ):
        for p in (pai_cert, dac_key, dac_cert):
            if not p.is_file():
                raise FileNotFoundError(f"Missing required file: {p}")

        self.write_data(
            storage,
            KeyType.ATTESTATION,
            {AttestationKeyId.KEY: self.read_key_file(dac_key)},
            wrap=wrap,
        )
        self.write_data(
            storage,
            KeyType.ATTESTATION,
            {
                AttestationKeyId.DAC: self.read_cert_file(dac_cert),
                AttestationKeyId.PAI: self.read_cert_file(pai_cert),
            },
        )

    def provision_setup(
        self, storage: CryptoStorage, passcode: int, discriminator: int
    ):
        salt, verifier = self.generate_spake2_values(passcode)
        self.write_data(
            storage,
            KeyType.SETUP,
            {
                SetupKeyID.SPAKE2P_SALT: salt,
                SetupKeyID.SPAKE2P_VERIFIER: verifier,
                SetupKeyID.SPAKE2P_ITER_COUNT: struct.pack("<I", 1000),
                SetupKeyID.DISCRIMINATOR: struct.pack("<H", discriminator),
                SetupKeyID.PASSCODE: struct.pack("<I", passcode),
            },
        )

    def provision_device_info(self, storage: CryptoStorage, serial: str):
        self.write_data(
            storage,
            KeyType.DEVICE_INFO,
            {
                DeviceInfoKeyID.VENDOR_ID: struct.pack("<H", 0x158A),
                DeviceInfoKeyID.PRODUCT_ID: struct.pack("<H", 0xBB01),
                DeviceInfoKeyID.VENDOR_NAME: to_terminated("Flipper FZCO"),
                DeviceInfoKeyID.PRODUCT_NAME: to_terminated("BUSY Bar"),
                DeviceInfoKeyID.PART_NUMBER: to_terminated("BB.1"),
                DeviceInfoKeyID.PRODUCT_URL: to_terminated("https://busy.bar"),
                DeviceInfoKeyID.PRODUCT_LABEL: to_terminated("BUSY Bar"),
                DeviceInfoKeyID.SERIAL_NUMBER: to_terminated(serial),
                DeviceInfoKeyID.MANUFACTURING_DATE: pack_current_date(),
            },
        )

    # -- main command -----------------------------------------------------

    @CatchExceptions
    def provision(self):
        insecure = self.args.insecure_crypto
        wrap = not insecure

        info = _get_device_info(self.get_portname())

        device_uid = info.get("u5_hardware_uid")
        if not device_uid:
            raise RuntimeError("Could not read u5_hardware_uid from device_info")

        if not insecure and info.get("sl_m4_secureboot") != "1":
            raise RuntimeError(
                "Key wrapping requested but device does not support secure boot"
                " (sl_m4_secureboot is not enabled). Pass --insecure-crypto to"
                " use plain key storage."
            )

        mode = "insecure" if insecure else "secure"
        certs_source = "production" if self.args.production_certs else "test"
        print(f"Matter provisioning [{mode}, {certs_source} certs] uid={device_uid}")

        with CryptoStorage(self.get_portname()) as storage:
            print("  Wiping crypto storage...")
            ret = storage.wipe_partition(Partition.MAIN, echo=False)
            if ret != 0:
                raise Exception(f"wipe_partition failed with error {ret}")

            if not self.args.no_attest:
                print("  Writing attestation (DAC key + certs)...")
                production_dir = (
                    self.args.production_certs.expanduser().resolve()
                    if self.args.production_certs
                    else None
                )
                if production_dir:
                    with _production_certs_bundle(production_dir) as (
                        pai_cert,
                        dac_key,
                        dac_cert,
                    ):
                        self.provision_attestation(
                            storage, pai_cert, dac_key, dac_cert, wrap
                        )
                else:
                    vid, pid = self.args.vendor_id, self.args.product_id
                    certs_dir = DEFAULT_CERTS_DIR
                    self.provision_attestation(
                        storage,
                        certs_dir / f"test-PAI-{vid}-cert.pem",
                        certs_dir / f"test-DAC-{vid}-{pid}-key.pem",
                        certs_dir / f"test-DAC-{vid}-{pid}-cert.pem",
                        wrap,
                    )

            if not self.args.no_setup:
                print("  Writing setup params (SPAKE2+, discriminator, passcode)...")
                passcode = self.normalize_numeric(self.args.passcode)
                discriminator = self.normalize_numeric(self.args.discriminator)
                self.provision_setup(storage, passcode, discriminator)

            if not self.args.no_info:
                print("  Writing device info...")
                self.provision_device_info(storage, device_uid)

        print(
            f"Matter provisioning OK"
            f"  passcode={self.args.passcode} discriminator={self.args.discriminator}"
        )


if __name__ == "__main__":
    Main()()
