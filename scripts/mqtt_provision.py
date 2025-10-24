#!/usr/bin/env python3

import os
import sys
from datetime import datetime, timedelta, timezone

import socket
import ssl
from typing import List

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import NameOID, ExtendedKeyUsageOID

from flipper.cli import Cli
from flipper.storage_socket import FlipperStorage, FlipperStorageOperations
from crypto_storage import CryptoStorage

MQTT_SERVER = "mqtt.cloud.dev.busy.app"
MQTT_PORT = 8883

CERTS_DIR = "scripts/test_certs/mqtt"

CA_BUNDLE = "ca_bundle.crt"
SIGN_CERT = "signing-ca.crt"
SIGN_KEY = "signing-ca.key"
DEVICE_CERT = "device.crt"
DEVICE_KEY = "device.key"

MQTT_DATA_DIR = "/ext/apps_assets/mqtt_client"

PORT_NAME = ("10.0.4.20", 23)
KEY_ID_TLS = 0x10
KEY_TYPE_ECDSA256 = 8


def write_certs():
    with FlipperStorage(PORT_NAME) as storage:
        if not storage.exist_dir(MQTT_DATA_DIR):
            storage.mkdir(MQTT_DATA_DIR)

        from_path = os.path.join(CERTS_DIR, CA_BUNDLE)
        to_path = MQTT_DATA_DIR + "/" + CA_BUNDLE
        print(f'Sending "{from_path}" to "{to_path}"')
        storage.send_file(f"{from_path}", f"{to_path}")

        from_path = os.path.join(CERTS_DIR, SIGN_CERT)
        to_path = MQTT_DATA_DIR + "/" + SIGN_CERT
        print(f'Sending "{from_path}" to "{to_path}"')
        storage.send_file(f"{from_path}", f"{to_path}")

        from_path = os.path.join(CERTS_DIR, DEVICE_CERT)
        to_path = MQTT_DATA_DIR + "/" + DEVICE_CERT
        print(f'Sending "{from_path}" to "{to_path}"')
        storage.send_file(f"{from_path}", f"{to_path}")


def write_private_key(key_file, wrap=False):
    with open(key_file, "rb") as f:
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
        storage.remove(MQTT_DATA_DIR + "/" + CA_BUNDLE)
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


def _collect_cert_chain(tls_socket: ssl.SSLSocket) -> List[bytes]:
    chain_bytes: List[bytes] = []
    chain_getters = [
        "getpeercertchain",
        "get_verified_chain",
        "get_unverified_chain",
    ]

    for getter_name in chain_getters:
        getter = getattr(tls_socket, getter_name, None)
        if callable(getter):
            try:
                chain = getter()
            except ssl.SSLError:
                continue
            if not chain:
                continue

            if isinstance(chain, (list, tuple)):
                candidate: List[bytes] = []
                for item in chain:
                    if isinstance(item, (bytes, bytearray, memoryview)):
                        candidate.append(bytes(item))
                if candidate:
                    chain_bytes = candidate
                    break
            elif isinstance(chain, (bytes, bytearray, memoryview)):
                chain_bytes = [bytes(chain)]
                break

    if not chain_bytes:
        leaf_cert = tls_socket.getpeercert(binary_form=True)
        if leaf_cert:
            chain_bytes = [leaf_cert]

    if not chain_bytes:
        raise RuntimeError("No certificates retrieved from TLS handshake")

    return chain_bytes


def get_ca_chain(hostname, port):
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    context.check_hostname = False
    context.verify_mode = ssl.CERT_NONE

    if hasattr(context, "minimum_version") and hasattr(ssl, "TLSVersion"):
        context.minimum_version = ssl.TLSVersion.TLSv1_3
        context.maximum_version = ssl.TLSVersion.TLSv1_3

    pem_chunks = []

    try:
        with socket.create_connection((hostname, port), timeout=5) as raw_sock:
            raw_sock.setblocking(True)
            with context.wrap_socket(raw_sock, server_hostname=hostname) as tls_sock:
                for cert_der in _collect_cert_chain(tls_sock):
                    pem_chunks.append(
                        ssl.DER_cert_to_PEM_cert(cert_der).encode("ascii")
                    )
    except (OSError, ssl.SSLError) as exc:
        raise RuntimeError(
            f"Failed to download certificate chain from {hostname}:{port}"
        ) from exc

    if not pem_chunks:
        raise RuntimeError("Certificate chain from server is empty")

    with open(os.path.join(CERTS_DIR, CA_BUNDLE), "wb") as f:
        for chunk in pem_chunks:
            f.write(chunk)


def gen_device_cert(device_uid):
    # Load signing CA
    with open(os.path.join(CERTS_DIR, SIGN_KEY), "rb") as f:
        ca_private_key = serialization.load_pem_private_key(f.read(), password=None)
        if not isinstance(ca_private_key, ec.EllipticCurvePrivateKey):
            raise TypeError("Signing CA key must be an elliptic-curve private key")
    with open(os.path.join(CERTS_DIR, SIGN_CERT), "rb") as f:
        ca_cert = x509.load_pem_x509_certificate(f.read())
        ca_public_key = ca_cert.public_key()
        if not isinstance(ca_public_key, ec.EllipticCurvePublicKey):
            raise TypeError(
                "Signing CA certificate must provide an elliptic-curve public key"
            )

    # Generate device private key
    device_private_key = ec.generate_private_key(ec.SECP256R1())
    with open(os.path.join(CERTS_DIR, DEVICE_KEY), "wb") as f:
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

    with open(os.path.join(CERTS_DIR, DEVICE_CERT), "wb") as f:
        f.write(device_cert.public_bytes(serialization.Encoding.PEM))


def main():
    if len(sys.argv) == 2 and sys.argv[1] == "-c":
        print("Cleanup")
        cleanup()
    else:
        device_uid = get_device_uid()
        print("UID:", device_uid)

        get_ca_chain(MQTT_SERVER, MQTT_PORT)
        gen_device_cert(device_uid)
        write_certs()
        write_private_key(os.path.join(CERTS_DIR, DEVICE_KEY), False)


if __name__ == "__main__":
    main()
