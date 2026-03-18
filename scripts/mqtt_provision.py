#!/usr/bin/env python3

from pathlib import Path
from datetime import datetime, timedelta, timezone

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import ExtendedKeyUsageOID

from flipper.cli import Cli
from flipper.app import App, CatchExceptions
from crypto_storage import CryptoStorage

CERTS_DIR_DEFAULT = Path("scripts/test_certs/mqtt")

SIGN_CERT = "signing-ca.crt"
SIGN_KEY = "signing-ca.key"
SIGN_CERT_DER = "signing-ca.der"
DEVICE_CERT = "device.der"

KEY_ID_OFFSET = 0x10
KEY_ID_TLS_SIGN = KEY_ID_OFFSET + 0  # Sign cert slot
KEY_ID_TLS_DEVICE = KEY_ID_OFFSET + 1  # Device cert + key slot

KEY_TYPE_ECDSA256_KEY = 8
KEY_TYPE_CSR_DER_ECDSA256 = 11
KEY_TYPE_ECDSA256_CERT = 12


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        # Provision command
        self.provision_parser = self.subparsers.add_parser(
            "provision", help="Generate device CSR, sign it, and write certs"
        )
        self.provision_parser.add_argument(
            "--certs-dir",
            type=Path,
            default=CERTS_DIR_DEFAULT,
            help="Directory containing CA certificate material",
        )
        self.provision_parser.set_defaults(func=self.provision)

        # Cleanup command
        self.cleanup_parser = self.subparsers.add_parser(
            "cleanup", help="Wipe key storage partition"
        )
        self.cleanup_parser.set_defaults(func=self.cleanup)

    def get_portname(self):
        return ("10.0.4.20", 23)

    def get_device_uid(self):
        with Cli(self.get_portname()) as cli:
            cli.send("device_info\r")
            cli.read.until("u5_hardware_uid")
            cli.read.until(": ")
            uid_str = cli.read.until(cli.CLI_EOL)
            cli.read.until(cli.CLI_PROMPT)
        return uid_str.decode("utf-8")

    def ensure_tls_slots_empty(self, crypto_storage: CryptoStorage):
        crypto_storage.ensure_key_absent(
            0,
            KEY_TYPE_ECDSA256_CERT,
            KEY_ID_TLS_SIGN,
            echo=True,
            error_message="TLS sign cert slot already provisioned; refusing to overwrite",
        )
        crypto_storage.ensure_key_absent(
            0,
            KEY_TYPE_ECDSA256_CERT,
            KEY_ID_TLS_DEVICE,
            echo=True,
            error_message="TLS device cert slot already provisioned; refusing to overwrite",
        )
        crypto_storage.ensure_key_absent(
            0,
            KEY_TYPE_ECDSA256_KEY,
            KEY_ID_TLS_DEVICE,
            echo=True,
            error_message="TLS device key slot already provisioned; refusing to overwrite",
        )

    def generate_device_csr(self, crypto_storage: CryptoStorage, device_uid: str):
        common_name = "BusyBar device " + device_uid
        subject_name = f"CN={common_name}," f"O=Flipper FZCO," f"C=AE"

        print(f"Generating key pair and CSR on device: {subject_name}")
        ret = crypto_storage.gen_csr(0, KEY_ID_TLS_DEVICE, 0, subject_name)
        if ret != 0:
            raise Exception(f"Device CSR generation failed with error {ret}")

        print("Reading CSR DER from device...")
        csr_der = crypto_storage.read_key_data(
            0, KEY_TYPE_CSR_DER_ECDSA256, KEY_ID_TLS_DEVICE
        )
        if csr_der is None:
            raise Exception("Failed to read CSR from device")

        return x509.load_der_x509_csr(csr_der)

    def sign_csr(self, certs_dir: Path, csr: x509.CertificateSigningRequest):
        with open(certs_dir / SIGN_KEY, "rb") as f:
            ca_private_key = serialization.load_pem_private_key(f.read(), password=None)
            if not isinstance(ca_private_key, ec.EllipticCurvePrivateKey):
                raise TypeError("Signing CA key must be an elliptic-curve private key")
        with open(certs_dir / SIGN_CERT, "rb") as f:
            ca_cert = x509.load_pem_x509_certificate(f.read())
            ca_public_key = ca_cert.public_key()
            if not isinstance(ca_public_key, ec.EllipticCurvePublicKey):
                raise TypeError(
                    "Signing CA certificate must provide an elliptic-curve public key"
                )

        device_cert = (
            x509.CertificateBuilder()
            .subject_name(csr.subject)
            .issuer_name(ca_cert.subject)
            .public_key(csr.public_key())
            .serial_number(x509.random_serial_number())
            .not_valid_before(datetime.now(timezone.utc))
            .not_valid_after(datetime.now(timezone.utc) + timedelta(days=36500))
            .add_extension(
                x509.KeyUsage(
                    digital_signature=True,
                    content_commitment=False,
                    key_encipherment=False,
                    data_encipherment=False,
                    key_agreement=False,
                    key_cert_sign=False,
                    crl_sign=False,
                    encipher_only=False,
                    decipher_only=False,
                ),
                critical=True,
            )
            .add_extension(
                x509.BasicConstraints(ca=False, path_length=None), critical=True
            )
            .add_extension(
                x509.ExtendedKeyUsage([ExtendedKeyUsageOID.CLIENT_AUTH]),
                critical=False,
            )
            .add_extension(
                x509.SubjectKeyIdentifier.from_public_key(csr.public_key()),
                critical=False,
            )
            .add_extension(
                x509.AuthorityKeyIdentifier.from_issuer_public_key(ca_public_key),
                critical=False,
            )
            .sign(ca_private_key, hashes.SHA256())
        )

        ca_cert_der = ca_cert.public_bytes(serialization.Encoding.DER)
        device_cert_der = device_cert.public_bytes(serialization.Encoding.DER)

        with open(certs_dir / SIGN_CERT_DER, "wb") as f:
            f.write(ca_cert_der)
        with open(certs_dir / DEVICE_CERT, "wb") as f:
            f.write(device_cert_der)

        return ca_cert_der, device_cert_der

    def write_certs(
        self, crypto_storage: CryptoStorage, ca_cert_der: bytes, device_cert_der: bytes
    ):
        ret = crypto_storage.write_key(
            0,
            KEY_TYPE_ECDSA256_CERT,
            KEY_ID_TLS_SIGN,
            0,
            len(ca_cert_der),
            ca_cert_der.hex(),
        )
        if ret != 0:
            raise Exception(f"write_key failed with error {ret}")

        ret = crypto_storage.write_key(
            0,
            KEY_TYPE_ECDSA256_CERT,
            KEY_ID_TLS_DEVICE,
            0,
            len(device_cert_der),
            device_cert_der.hex(),
        )
        if ret != 0:
            raise Exception(f"write_key failed with error {ret}")

    @CatchExceptions
    def provision(self):
        certs_dir = self.args.certs_dir.expanduser()
        device_uid = self.get_device_uid()
        print("UID:", device_uid)

        with CryptoStorage(self.get_portname()) as crypto_storage:
            self.ensure_tls_slots_empty(crypto_storage)
            csr = self.generate_device_csr(crypto_storage, device_uid)
            ca_cert_der, device_cert_der = self.sign_csr(certs_dir, csr)
            self.write_certs(crypto_storage, ca_cert_der, device_cert_der)

    @CatchExceptions
    def cleanup(self):
        with CryptoStorage(self.get_portname()) as crypto_storage:
            ret = crypto_storage.wipe_partition(0)
            if ret != 0:
                raise Exception(f"wipe_partition failed with error {ret}")


if __name__ == "__main__":
    Main()()
