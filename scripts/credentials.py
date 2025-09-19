#!/usr/bin/env python3

import os
import struct
import hashlib

from enum import IntEnum

from random import randbytes, randint

from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization

from ecdsa.curves import NIST256p

from flipper.app import App, CatchExceptions
from crypto_storage import CryptoStorage


class Partition(IntEnum):
    MAIN = 0
    USER = 1


class KeyType(IntEnum):
    PRIVATE_KEY = 8
    DAC = 13
    PAI = 14
    CD = 15
    SETUP = 16
    DEVICE_INFO = 17


class KeyID(IntEnum):
    DEFAULT = 0


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

        # DAC command
        self.dac_parser = self.subparsers.add_parser(
            "dac", help="Provision device attestation certificate (DAC)"
        )
        self.dac_parser.add_argument("filename", help="DAC file (.pem or .der format)")
        self.dac_parser.set_defaults(func=self.provision_dac)

        # PAI command
        self.pai_parser = self.subparsers.add_parser(
            "pai", help="Provision product attestation intermediate cert (PAI)"
        )
        self.pai_parser.add_argument("filename", help="PAI file (.pem or .der format)")
        self.pai_parser.set_defaults(func=self.provision_pai)

        # CD command
        self.cd_parser = self.subparsers.add_parser(
            "cd", help="Provision product cerfification declaration (CD)"
        )
        self.cd_parser.add_argument("filename", help="CD file (.der format)")
        self.cd_parser.set_defaults(func=self.provision_cd)

        # Private key command
        self.pk_parser = self.subparsers.add_parser(
            "pk", help="Provision device attestation private key"
        )
        self.pk_parser.add_argument(
            "filename", help="Private key file (.pem or .der format)"
        )
        self.pk_parser.set_defaults(func=self.provision_private_key)

        # Setup command
        self.setup_parser = self.subparsers.add_parser(
            "setup", help="Provision setup discriminator"
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
        self, passcode: int, iter_count: int
    ) -> tuple[bytes, bytes]:
        salt = randbytes(randint(16, 32))  # Salt must be between 16 and 32 bytes long
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

    def write_data(self, key_type: int, data: dict[int, bytes]):
        with CryptoStorage(self.get_portname()) as storage:
            for key_id, key_value in data.items():
                ret = storage.write_key(
                    Partition.MAIN, key_type, key_id, 0, len(key_value), key_value.hex()
                )
                if ret != 0:
                    raise Exception(f"write_key failed with error {ret}")

    @CatchExceptions
    def provision_dac(self):
        data = {KeyID.DEFAULT: self.read_cert_file(self.args.filename)}
        self.write_data(KeyType.DAC, data)

    @CatchExceptions
    def provision_pai(self):
        data = {KeyID.DEFAULT: self.read_cert_file(self.args.filename)}
        self.write_data(KeyType.PAI, data)

    @CatchExceptions
    def provision_cd(self):
        data = {KeyID.DEFAULT: self.read_cert_file(self.args.filename)}
        self.write_data(KeyType.CD, data)

    @CatchExceptions
    def provision_private_key(self):
        data = {KeyID.DEFAULT: self.read_key_file(self.args.filename)}
        self.write_data(KeyType.PRIVATE_KEY, data)

    @CatchExceptions
    def provision_setup_params(self):
        iter_count = 1000
        salt, verifier = self.generate_spake2_values(self.args.passcode, iter_count)
        data = {
            SetupKeyID.SPAKE2P_SALT: salt,
            SetupKeyID.SPAKE2P_VERIFIER: verifier,
            SetupKeyID.SPAKE2P_ITER_COUNT: struct.pack("<I", iter_count),
            SetupKeyID.DISCRIMINATOR: struct.pack("<H", self.args.discriminator),
            SetupKeyID.PASSCODE: struct.pack("<I", self.args.passcode),
        }
        self.write_data(KeyType.SETUP, data)

    def get_portname(self):
        return ("10.0.4.20", 23)


if __name__ == "__main__":
    Main()()
