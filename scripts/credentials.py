#!/usr/bin/env python3

import os
import struct
import hashlib

from enum import IntEnum

from random import randbytes

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
    VID_PID = 16
    SPAKE2_SALT = 17
    SPAKE2_VERIFIER = 18
    DISCRIMINATOR = 19
    PASSCODE = 20


class KeyID(IntEnum):
    DEFAULT = 0


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

        # SPAKE2 command
        self.spake2_parser = self.subparsers.add_parser(
            "spake2", help="Generate and provision SPAKE2 values"
        )
        self.spake2_parser.add_argument(
            "passcode", type=int, help="Passcode for device pairing"
        )
        self.spake2_parser.set_defaults(func=self.provision_spake2)

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

    def generate_spake2_values(self, passcode: int) -> tuple[bytes, bytes]:
        num_iter = 1000
        salt = randbytes(32)
        ws_len = NIST256p.baselen + 8

        ws = hashlib.pbkdf2_hmac(
            "sha256", struct.pack("<I", passcode), salt, num_iter, ws_len * 2
        )

        w0 = int.from_bytes(ws[:ws_len], byteorder="big") % NIST256p.order
        w1 = int.from_bytes(ws[ws_len:], byteorder="big") % NIST256p.order
        L = NIST256p.generator * w1

        verifier = w0.to_bytes(NIST256p.baselen, byteorder="big") + L.to_bytes(
            "uncompressed"
        )

        return (salt, verifier)

    def write_data(self, key_type: int, data: bytes):
        with CryptoStorage(self.get_portname()) as storage:
            ret = storage.write_key(
                Partition.MAIN, key_type, KeyID.DEFAULT, 0, len(data), data.hex()
            )
        if ret != 0:
            raise Exception(f"write_key failed with error {ret}")

    @CatchExceptions
    def provision_dac(self):
        data = self.read_cert_file(self.args.filename)
        self.write_data(KeyType.DAC, data)

    @CatchExceptions
    def provision_pai(self):
        data = self.read_cert_file(self.args.filename)
        self.write_data(KeyType.PAI, data)

    @CatchExceptions
    def provision_cd(self):
        data = self.read_cert_file(self.args.filename)
        self.write_data(KeyType.CD, data)

    @CatchExceptions
    def provision_private_key(self):
        data = self.read_key_file(self.args.filename)
        self.write_data(KeyType.PRIVATE_KEY, data)

    @CatchExceptions
    def provision_spake2(self):
        salt, verifier = self.generate_spake2_values(self.args.passcode)
        self.write_data(KeyType.SPAKE2_SALT, salt)
        self.write_data(KeyType.SPAKE2_VERIFIER, verifier)

    @CatchExceptions
    def provision_setup_params(self):
        discriminator_bytes = struct.pack("<H", self.args.discriminator)
        self.write_data(KeyType.DISCRIMINATOR, discriminator_bytes)
        passcode_bytes = struct.pack("<I", self.args.passcode)
        self.write_data(KeyType.PASSCODE, passcode_bytes)

    def get_portname(self):
        return ("10.0.4.20", 23)


if __name__ == "__main__":
    Main()()
