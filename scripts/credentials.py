#!/usr/bin/env python3

import os

from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization

from flipper.app import App, CatchExceptions
from crypto_storage import CryptoStorage


class Main(App):
    PART_MAIN = 0
    KEY_TYPE_PK = 8
    KEY_TYPE_DAC = 13
    KEY_TYPE_PAI = 14
    KEY_TYPE_CD = 15
    KEY_ID_DEFAULT = 0

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
                data = fd.read()
        elif ext == ".pem":
            with open(filename, "rb") as fd:
                key = serialization.load_pem_private_key(
                    fd.read(), None, default_backend()
                )
                data = key.private_bytes(
                    serialization.Encoding.DER,
                    serialization.PrivateFormat.TraditionalOpenSSL,
                    serialization.NoEncryption(),
                )
        else:
            raise Exception("Please choose a .pem or .der file")

        return data

    def write_data(self, key_type: int, data: bytes):
        with CryptoStorage(self.get_portname()) as storage:
            ret = storage.write_key(
                self.PART_MAIN, key_type, self.KEY_ID_DEFAULT, 0, len(data), data.hex()
            )
        if ret != 0:
            raise Exception(f"write_key failed with error {ret}")

    @CatchExceptions
    def provision_dac(self):
        data = self.read_cert_file(self.args.filename)
        self.write_data(self.KEY_TYPE_DAC, data)

    @CatchExceptions
    def provision_pai(self):
        data = self.read_cert_file(self.args.filename)
        self.write_data(self.KEY_TYPE_PAI, data)

    @CatchExceptions
    def provision_cd(self):
        data = self.read_cert_file(self.args.filename)
        self.write_data(self.KEY_TYPE_CD, data)

    @CatchExceptions
    def provision_private_key(self):
        data = self.read_key_file(self.args.filename)
        self.write_data(self.KEY_TYPE_PK, data)

    def get_portname(self):
        return ("10.0.4.20", 23)


if __name__ == "__main__":
    Main()()
