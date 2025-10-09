#!/usr/bin/env python3

import os
import struct
import hashlib

from enum import IntEnum, IntFlag
from datetime import datetime
from random import randbytes

from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization

from ecdsa.curves import NIST256p

from flipper.app import App, CatchExceptions
from crypto_storage import CryptoStorage


def auto_int(x):
    return int(x, 0)


def to_terminated(s: str) -> bytes:
    return s.encode("ascii") + b"\x00"


def pack_current_date() -> bytes:
    now = datetime.today()
    return struct.pack("<HBB", now.year, now.month, now.day)


class Partition(IntEnum):
    MAIN = 0
    USER = 1


class KeyType(IntEnum):
    ATTESTATION = 13
    SETUP = 14
    DEVICE_INFO = 15


class WriteFlag(IntFlag):
    NONE = 0
    WRAP = 1


class AttestationKeyId(IntEnum):
    KEY = 0
    DAC = 1
    PAI = 2
    CD = 3


class SetupKeyID(IntEnum):
    SPAKE2P_SALT = 0
    SPAKE2P_VERIFIER = 1
    SPAKE2P_ITER_COUNT = 2
    DISCRIMINATOR = 3
    PASSCODE = 4


class DeviceInfoKeyID(IntEnum):
    VENDOR_ID = 0
    PRODUCT_ID = 1
    VENDOR_NAME = 2
    PRODUCT_NAME = 3
    PART_NUMBER = 4
    PRODUCT_URL = 5
    PRODUCT_LABEL = 6
    SERIAL_NUMBER = 7
    MANUFACTURING_DATE = 8
    HARDWARE_VERSION = 9
    HARDWARE_VERSION_STRING = 10


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        # Attest command
        self.attest_parser = self.subparsers.add_parser(
            "attest", help="Provision device attestation files"
        )
        self.attest_parser.add_argument(
            "--key", required=True, help="Private key file (.pem or .der format)"
        )
        self.attest_parser.add_argument(
            "--dac",
            required=True,
            help="DAC (Device Attestation Certificate) file (.pem or .der format)",
        )
        self.attest_parser.add_argument(
            "--pai",
            required=True,
            help="PAI (Product Attestation Intermediate) file (.pem or .der format)",
        )
        self.attest_parser.add_argument(
            "--cd",
            required=True,
            help="CD (Certification Declaration) file (.der format)",
        )
        self.attest_parser.add_argument(
            "--wrap-private-key",
            action="store_true",
            help="Wrap private key with device internal key",
        )

        self.attest_parser.set_defaults(func=self.provision_attestation_files)

        # Setup command
        self.setup_parser = self.subparsers.add_parser(
            "setup", help="Provision setup parameters"
        )
        self.setup_parser.add_argument(
            "-l", "--salt-length", type=int, default=32, help="SPAKE2 salt length"
        )
        self.setup_parser.add_argument(
            "-i",
            "--iteration-count",
            type=int,
            default=1000,
            help="SPAKE2 iterarion count",
        )
        self.setup_parser.add_argument(
            "-d",
            "--discriminator",
            required=True,
            type=int,
            help="Setup discriminator value",
        )
        self.setup_parser.add_argument(
            "-p", "--passcode", required=True, type=int, help="Setup passcode value"
        )
        self.setup_parser.set_defaults(func=self.provision_setup_params)

        # Device Info command
        self.info_parser = self.subparsers.add_parser(
            "info", help="Provision device info"
        )
        self.info_parser.add_argument(
            "--vid", type=auto_int, default=0x158A, help="Numeric vendor ID"
        )
        self.info_parser.add_argument(
            "--pid", type=auto_int, default=0x001, help="Numeric product ID"
        )
        self.info_parser.add_argument(
            "--vendor-name",
            type=str,
            default="Flipper Devices Inc",
            help="Vendor name string",
        )
        self.info_parser.add_argument(
            "--product-name",
            type=str,
            default="BUSY Bar",
            help="Product name string",
        )
        self.info_parser.add_argument(
            "--part-number", type=str, default="BSB0001", help="Product part number"
        )
        self.info_parser.add_argument(
            "--product-url",
            type=str,
            default="https://busy.bar",
            help="Product homepage",
        )
        self.info_parser.add_argument(
            "--product-label",
            type=str,
            default="BUSY",
            help="Product label (shown in app)",
        )
        self.info_parser.add_argument(
            "--serial-number",
            type=str,
            default="1234567890",
            help="Device serial number",
        )
        self.info_parser.add_argument(
            "--hardware-version", type=int, default=0, help="Device hardware version"
        )
        self.info_parser.add_argument(
            "--hardware-version-string",
            type=str,
            default="Version 0",
            help="Device hardware version string",
        )
        self.info_parser.set_defaults(func=self.provision_info)

    def read_cert_file(self, filename: str) -> bytes:
        _, ext = os.path.splitext(filename)

        if ext == ".der":
            with open(filename, "rb") as fd:
                data = fd.read()
        elif ext == ".pem":
            with open(filename, "rb") as fd:
                cert = x509.load_pem_x509_certificate(fd.read(), default_backend())
                data = cert.public_bytes(serialization.Encoding.DER)
        else:
            raise Exception("Please choose a .pem or .der file")

        return data

    def read_key_file(self, filename: str) -> bytes:
        _, ext = os.path.splitext(filename)

        if ext == ".der":
            with open(filename, "rb") as fd:
                key = serialization.load_der_private_key(
                    fd.read(), None, default_backend()
                )
        elif ext == ".pem":
            with open(filename, "rb") as fd:
                key = serialization.load_pem_private_key(
                    fd.read(), None, default_backend()
                )
        else:
            raise Exception("Please choose a .pem or .der file")

        return key.private_numbers().private_value.to_bytes(32, "big")

    def generate_spake2_values(
        self, passcode: int, salt_len: int, iter_count: int
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

        return (salt, verifier)

    def write_data(self, key_type: int, data: dict[int, bytes], wrap=False):
        with CryptoStorage(self.get_portname()) as storage:
            for key_id, key_value in data.items():
                flags = WriteFlag.WRAP if wrap else WriteFlag.NONE
                ret = storage.write_key(
                    Partition.MAIN,
                    key_type,
                    key_id,
                    flags,
                    len(key_value),
                    key_value.hex(),
                )
                if ret != 0:
                    raise Exception(f"write_key failed with error {ret}")

    @CatchExceptions
    def provision_attestation_files(self):
        data = {
            AttestationKeyId.KEY: self.read_key_file(self.args.key),
            AttestationKeyId.DAC: self.read_cert_file(self.args.dac),
            AttestationKeyId.PAI: self.read_cert_file(self.args.pai),
            AttestationKeyId.CD: self.read_cert_file(self.args.cd),
        }
        self.write_data(KeyType.ATTESTATION, data)

    @CatchExceptions
    def provision_setup_params(self):
        salt, verifier = self.generate_spake2_values(
            self.args.passcode, self.args.salt_length, self.args.iteration_count
        )
        data = {
            SetupKeyID.SPAKE2P_SALT: salt,
            SetupKeyID.SPAKE2P_VERIFIER: verifier,
            SetupKeyID.SPAKE2P_ITER_COUNT: struct.pack("<I", self.args.iteration_count),
            SetupKeyID.DISCRIMINATOR: struct.pack("<H", self.args.discriminator),
            SetupKeyID.PASSCODE: struct.pack("<I", self.args.passcode),
        }
        self.write_data(KeyType.SETUP, data)

    @CatchExceptions
    def provision_info(self):
        data = {
            DeviceInfoKeyID.VENDOR_ID: struct.pack("<H", self.args.vid),
            DeviceInfoKeyID.PRODUCT_ID: struct.pack("<H", self.args.pid),
            DeviceInfoKeyID.VENDOR_NAME: to_terminated(self.args.vendor_name),
            DeviceInfoKeyID.PRODUCT_NAME: to_terminated(self.args.product_name),
            DeviceInfoKeyID.PART_NUMBER: to_terminated(self.args.part_number),
            DeviceInfoKeyID.PRODUCT_URL: to_terminated(self.args.product_url),
            DeviceInfoKeyID.PRODUCT_LABEL: to_terminated(self.args.product_label),
            DeviceInfoKeyID.SERIAL_NUMBER: to_terminated(self.args.serial_number),
            DeviceInfoKeyID.MANUFACTURING_DATE: pack_current_date(),
            DeviceInfoKeyID.HARDWARE_VERSION: struct.pack(
                "<H", self.args.hardware_version
            ),
            DeviceInfoKeyID.HARDWARE_VERSION_STRING: to_terminated(
                self.args.hardware_version_string
            ),
        }
        self.write_data(KeyType.DEVICE_INFO, data)

    def get_portname(self):
        return ("10.0.4.20", 23)


if __name__ == "__main__":
    Main()()
