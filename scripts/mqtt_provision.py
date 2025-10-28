#!/usr/bin/env python3

import argparse
import sys
from pathlib import Path
from datetime import datetime, timedelta, timezone

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import NameOID, ExtendedKeyUsageOID

from flipper.cli import Cli
from flipper.storage_socket import FlipperStorage
from crypto_storage import CryptoStorage

CERTS_DIR_DEFAULT = Path("scripts/test_certs/mqtt")

SIGN_CERT = "signing-ca.crt"
SIGN_KEY = "signing-ca.key"
DEVICE_CERT = "device.crt"
DEVICE_KEY = "device.key"

MQTT_DATA_DIR = "/ext/apps_assets/mqtt_client"

PORT_NAME = ("10.0.4.20", 23)
KEY_ID_TLS = 0x10
KEY_TYPE_ECDSA256 = 8


def write_certs(certs_dir: Path):
    with FlipperStorage(PORT_NAME) as storage:
        if not storage.exist_dir(MQTT_DATA_DIR):
            storage.mkdir(MQTT_DATA_DIR)

        from_path = certs_dir / SIGN_CERT
        to_path = MQTT_DATA_DIR + "/" + SIGN_CERT
        print(f'Sending "{from_path}" to "{to_path}"')
        storage.send_file(str(from_path), f"{to_path}")

        from_path = certs_dir / DEVICE_CERT
        to_path = MQTT_DATA_DIR + "/" + DEVICE_CERT
        print(f'Sending "{from_path}" to "{to_path}"')
        storage.send_file(str(from_path), f"{to_path}")


def write_private_key(key_file, wrap=False):
    key_path = Path(key_file)

    with open(key_path, "rb") as f:
        private_key = serialization.load_pem_private_key(f.read(), password=None)
        if not isinstance(private_key, ec.EllipticCurvePrivateKey):
            raise TypeError("Expected an elliptic-curve private key for TLS storage")

        private_numbers = private_key.private_numbers()
        key_data = private_numbers.private_value.to_bytes(
            (private_numbers.private_value.bit_length() + 7) // 8,
            byteorder="big",
        )

    with CryptoStorage(PORT_NAME) as crypto_storage:
        flags = 1 if wrap else 0

        ret = crypto_storage.list_partition(0)
        if ret != 0:
            raise Exception(f"list_partition failed with error {ret}")
        # TODO: check if key exists

        ret = crypto_storage.write_key(
            0,
            KEY_TYPE_ECDSA256,
            KEY_ID_TLS,
            flags,
            len(key_data),
            key_data.hex(),
        )
        if ret != 0:
            raise Exception(f"write_key failed with error {ret}")


def cleanup():
    with FlipperStorage(PORT_NAME) as storage:
        storage.remove(MQTT_DATA_DIR + "/" + SIGN_CERT)
        storage.remove(MQTT_DATA_DIR + "/" + DEVICE_CERT)

    with CryptoStorage(PORT_NAME) as crypto_storage:
        ret = crypto_storage.wipe_partition(0)
        if ret != 0:
            raise Exception(f"wipe_partition failed with error {ret}")


def get_device_uid():
    with Cli(PORT_NAME) as cli:
        cli.send("device_info\r")
        cli.read.until("u5_hardware_uid")
        cli.read.until(": ")
        uid_str = cli.read.until(cli.CLI_EOL)
        cli.read.until(cli.CLI_PROMPT)
    return uid_str.decode("utf-8")


def gen_device_cert(certs_dir: Path, device_uid):
    # Load signing CA
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

    # Generate device private key
    device_private_key = ec.generate_private_key(ec.SECP256R1())
    with open(certs_dir / DEVICE_KEY, "wb") as f:
        f.write(
            device_private_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.TraditionalOpenSSL,
                encryption_algorithm=serialization.NoEncryption(),
            )
        )

    # Device CSR
    common_name = "BusyBar device " + device_uid
    device_subject = x509.Name(
        [
            x509.NameAttribute(NameOID.COUNTRY_NAME, "US"),
            x509.NameAttribute(NameOID.STATE_OR_PROVINCE_NAME, "Delaware"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "Flipper Devices Inc"),
            x509.NameAttribute(NameOID.COMMON_NAME, common_name),
        ]
    )
    csr = (
        x509.CertificateSigningRequestBuilder()
        .subject_name(device_subject)
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
            x509.ExtendedKeyUsage([ExtendedKeyUsageOID.CLIENT_AUTH]), critical=False
        )
        .sign(device_private_key, hashes.SHA256())
    )

    # Sign CSR
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
            critical=True,  # False?
        )
        .add_extension(x509.BasicConstraints(ca=False, path_length=None), critical=True)
        .add_extension(
            x509.ExtendedKeyUsage([ExtendedKeyUsageOID.CLIENT_AUTH]), critical=False
        )
        .add_extension(
            x509.SubjectKeyIdentifier.from_public_key(csr.public_key()), critical=False
        )
        .add_extension(
            x509.AuthorityKeyIdentifier.from_issuer_public_key(ca_public_key),
            critical=False,
        )
        .sign(ca_private_key, hashes.SHA256())
    )

    with open(certs_dir / DEVICE_CERT, "wb") as f:
        f.write(device_cert.public_bytes(serialization.Encoding.PEM))


def parse_args(argv):
    parser = argparse.ArgumentParser(description="Provision MQTT credentials")
    parser.add_argument(
        "-c",
        "--cleanup",
        action="store_true",
        help="Remove certificates from device and wipe key storage",
    )
    parser.add_argument(
        "--certs-dir",
        type=Path,
        default=CERTS_DIR_DEFAULT,
        help="Directory containing certificate material",
    )
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(sys.argv[1:] if argv is None else argv)
    certs_dir = args.certs_dir.expanduser()

    if args.cleanup:
        print("Cleanup")
        cleanup()
        return

    device_uid = get_device_uid()
    print("UID:", device_uid)

    gen_device_cert(certs_dir, device_uid)
    write_certs(certs_dir)
    write_private_key(certs_dir / DEVICE_KEY, False)


if __name__ == "__main__":
    main()
