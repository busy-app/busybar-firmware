#!/usr/bin/env python3

from pathlib import Path
from datetime import datetime, timedelta, timezone

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import NameOID, ExtendedKeyUsageOID

from flipper.cli import Cli
from flipper.app import App, CatchExceptions
from crypto_storage import CryptoStorage, ReadWriteScope

CERTS_DIR_DEFAULT = Path("scripts/test_certs/mqtt")

SIGN_CERT = "signing-ca.crt"
SIGN_KEY = "signing-ca.key"
SIGN_CERT_DER = "signing-ca.der"
DEVICE_CERT = "device.der"
DEVICE_KEY = "device.key"

KEY_ID_OFFSET = 0x10
KEY_ID_TLS_SIGN = KEY_ID_OFFSET + 0  # Sign cert slot
KEY_ID_TLS_DEVICE = KEY_ID_OFFSET + 1  # Device cert + key slot

KEY_TYPE_ECDSA256_KEY = 8
KEY_TYPE_CSR_DER_ECDSA256 = 11
KEY_TYPE_ECDSA256_CERT = 12


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


def _load_ca_material(certs_dir: Path):
    """Load and validate CA private key and certificate."""
    with open(certs_dir / SIGN_KEY, "rb") as f:
        ca_key = serialization.load_pem_private_key(f.read(), password=None)
        if not isinstance(ca_key, ec.EllipticCurvePrivateKey):
            raise TypeError("Signing CA key must be an elliptic-curve private key")
    with open(certs_dir / SIGN_CERT, "rb") as f:
        ca_cert = x509.load_pem_x509_certificate(f.read())
        ca_pub = ca_cert.public_key()
        if not isinstance(ca_pub, ec.EllipticCurvePublicKey):
            raise TypeError(
                "Signing CA certificate must provide an elliptic-curve public key"
            )
    return ca_key, ca_cert


def _build_device_cert(
    ca_key: ec.EllipticCurvePrivateKey,
    ca_cert: x509.Certificate,
    csr: x509.CertificateSigningRequest,
) -> x509.Certificate:
    """Sign a CSR into a device certificate."""
    ca_pub = ca_cert.public_key()
    return (
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
        .add_extension(x509.BasicConstraints(ca=False, path_length=None), critical=True)
        .add_extension(
            x509.ExtendedKeyUsage([ExtendedKeyUsageOID.CLIENT_AUTH]),
            critical=False,
        )
        .add_extension(
            x509.SubjectKeyIdentifier.from_public_key(csr.public_key()),
            critical=False,
        )
        .add_extension(
            x509.AuthorityKeyIdentifier.from_issuer_public_key(ca_pub),
            critical=False,
        )
        .sign(ca_key, hashes.SHA256())
    )


def _make_subject(device_uid: str) -> x509.Name:
    return x509.Name(
        [
            x509.NameAttribute(NameOID.COMMON_NAME, f"BusyBar device {device_uid}"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "Flipper FZCO"),
            x509.NameAttribute(NameOID.COUNTRY_NAME, "AE"),
        ]
    )


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        # Provision command
        self.provision_parser = self.subparsers.add_parser(
            "provision", help="Provision MQTT TLS credentials"
        )
        self.provision_parser.add_argument(
            "--certs-dir",
            type=Path,
            default=CERTS_DIR_DEFAULT,
            help="Directory containing CA certificate material",
        )
        self.provision_parser.add_argument(
            "--insecure-crypto",
            action="store_true",
            default=False,
            help="Generate key pair on host and store without wrapping"
            " (for devices without secure boot)",
        )
        self.provision_parser.set_defaults(func=self.provision)

        # Import command
        self.import_parser = self.subparsers.add_parser(
            "import",
            help="Import existing device cert chain (device.crt: device + CA PEM)"
            " and private key (device.key) instead of generating new ones",
        )
        self.import_parser.add_argument(
            "--certs-dir",
            type=Path,
            default=CERTS_DIR_DEFAULT,
            help="Directory containing device.crt (device + CA chain) and device.key",
        )
        self.import_parser.set_defaults(func=self.import_creds)

        # Cleanup command
        self.cleanup_parser = self.subparsers.add_parser(
            "cleanup", help="Wipe key storage partition"
        )
        self.cleanup_parser.set_defaults(func=self.cleanup)

    def get_portname(self):
        return ("10.0.4.20", 23)

    # -- helpers ----------------------------------------------------------

    def ensure_tls_slots_empty(self, crypto_storage: CryptoStorage):
        keys, _listing, ret = crypto_storage.enumerate_keys(0, echo=False)
        if ret != 0:
            raise Exception(f"list_partition failed with error {ret}")

        occupied = {
            (KEY_TYPE_ECDSA256_CERT, KEY_ID_TLS_SIGN): "TLS sign cert",
            (KEY_TYPE_ECDSA256_CERT, KEY_ID_TLS_DEVICE): "TLS device cert",
            (KEY_TYPE_ECDSA256_KEY, KEY_ID_TLS_DEVICE): "TLS device key",
        }
        for entry in keys:
            label = occupied.get((entry.key_type, entry.key_id))
            if label and entry.partition == 0:
                raise RuntimeError(
                    f"{label} slot already provisioned; refusing to overwrite"
                )

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
            echo=False,
        )
        if ret != 0:
            raise Exception(f"write_key (CA cert) failed with error {ret}")

        ret = crypto_storage.write_key(
            0,
            KEY_TYPE_ECDSA256_CERT,
            KEY_ID_TLS_DEVICE,
            0,
            len(device_cert_der),
            device_cert_der.hex(),
            echo=False,
        )
        if ret != 0:
            raise Exception(f"write_key (device cert) failed with error {ret}")

    def write_private_key(
        self, crypto_storage: CryptoStorage, key_data: bytes, wrap: bool
    ):
        flags = 1 if wrap else 0
        ret = crypto_storage.write_key(
            0,
            KEY_TYPE_ECDSA256_KEY,
            KEY_ID_TLS_DEVICE,
            flags,
            len(key_data),
            key_data.hex(),
            echo=False,
        )
        if ret != 0:
            raise Exception(f"write_key (private key) failed with error {ret}")

    # -- secure path: CSR generated on device, key never leaves it --------

    def provision_secure(
        self, crypto_storage: CryptoStorage, certs_dir: Path, device_uid: str
    ):
        subject = f"CN=BusyBar device {device_uid},O=Flipper FZCO,C=AE"
        print(f"  Generating wrapped key pair + CSR on device...")
        ret = crypto_storage.gen_csr(0, KEY_ID_TLS_DEVICE, 0, subject, echo=False)
        if ret != 0:
            raise Exception(f"Device CSR generation failed with error {ret}")

        csr_der = crypto_storage.read_key_data(
            0, KEY_TYPE_CSR_DER_ECDSA256, KEY_ID_TLS_DEVICE
        )
        if csr_der is None:
            raise Exception("Failed to read CSR from device")

        csr = x509.load_der_x509_csr(csr_der)
        ca_key, ca_cert = _load_ca_material(certs_dir)
        device_cert = _build_device_cert(ca_key, ca_cert, csr)

        print(f"  Writing CA + device certs...")
        ca_cert_der = ca_cert.public_bytes(serialization.Encoding.DER)
        device_cert_der = device_cert.public_bytes(serialization.Encoding.DER)
        self.write_certs(crypto_storage, ca_cert_der, device_cert_der)

    # -- insecure path: key pair generated on host, written to device -----

    def provision_insecure(
        self, crypto_storage: CryptoStorage, certs_dir: Path, device_uid: str
    ):
        print(f"  Generating key pair on host...")
        device_private_key = ec.generate_private_key(ec.SECP256R1())

        # Save PEM copy for debugging / backup
        with open(certs_dir / DEVICE_KEY, "wb") as f:
            f.write(
                device_private_key.private_bytes(
                    encoding=serialization.Encoding.PEM,
                    format=serialization.PrivateFormat.TraditionalOpenSSL,
                    encryption_algorithm=serialization.NoEncryption(),
                )
            )

        csr = (
            x509.CertificateSigningRequestBuilder()
            .subject_name(_make_subject(device_uid))
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
                x509.ExtendedKeyUsage([ExtendedKeyUsageOID.CLIENT_AUTH]),
                critical=False,
            )
            .sign(device_private_key, hashes.SHA256())
        )

        ca_key, ca_cert = _load_ca_material(certs_dir)
        device_cert = _build_device_cert(ca_key, ca_cert, csr)

        print(f"  Writing CA + device certs...")
        ca_cert_der = ca_cert.public_bytes(serialization.Encoding.DER)
        device_cert_der = device_cert.public_bytes(serialization.Encoding.DER)
        self.write_certs(crypto_storage, ca_cert_der, device_cert_der)

        print(f"  Writing private key (unwrapped)...")
        # Write raw private scalar to device (unwrapped)
        pn = device_private_key.private_numbers()
        key_data = pn.private_value.to_bytes(32, "big")
        self.write_private_key(crypto_storage, key_data, wrap=False)

    # -- commands ---------------------------------------------------------

    @CatchExceptions
    def provision(self):
        certs_dir = self.args.certs_dir.expanduser()
        insecure = self.args.insecure_crypto

        info = _get_device_info(self.get_portname())

        device_uid = info.get("u5_hardware_uid")
        if not device_uid:
            raise RuntimeError("Could not read u5_hardware_uid from device_info")

        if not insecure and info.get("sl_m4_secureboot") != "true":
            raise RuntimeError(
                "Key wrapping requested but device does not support secure boot"
                " (sl_m4_secureboot is not enabled). Pass --insecure-crypto to"
                " use plain key storage."
            )

        mode = "insecure" if insecure else "secure"
        print(f"MQTT TLS provisioning [{mode}] uid={device_uid}")

        with CryptoStorage(self.get_portname()) as crypto_storage:
            with ReadWriteScope(crypto_storage):
                print(f"  Checking TLS slots are empty...")
                self.ensure_tls_slots_empty(crypto_storage)
                if insecure:
                    self.provision_insecure(crypto_storage, certs_dir, device_uid)
                else:
                    self.provision_secure(crypto_storage, certs_dir, device_uid)
        print("MQTT TLS provisioning OK")

    @CatchExceptions
    def import_creds(self):
        certs_dir = self.args.certs_dir.expanduser()

        with open(certs_dir / "device.crt", "rb") as f:
            chain = x509.load_pem_x509_certificates(f.read())
        if len(chain) != 2:
            raise RuntimeError(
                f"device.crt must contain device + CA chain, got {len(chain)} cert(s)"
            )
        device_cert, ca_cert = chain
        if device_cert.issuer != ca_cert.subject:
            raise RuntimeError("device.crt chain mismatch: CA is not the issuer")

        with open(certs_dir / DEVICE_KEY, "rb") as f:
            device_key = serialization.load_pem_private_key(f.read(), password=None)
        if not isinstance(device_key, ec.EllipticCurvePrivateKey):
            raise TypeError("device.key must be an elliptic-curve private key")

        print(f"MQTT TLS import: {device_cert.subject.rfc4514_string()}")
        with CryptoStorage(self.get_portname()) as crypto_storage:
            with ReadWriteScope(crypto_storage):
                print("  Checking TLS slots are empty...")
                self.ensure_tls_slots_empty(crypto_storage)
                print("  Writing CA + device certs...")
                self.write_certs(
                    crypto_storage,
                    ca_cert.public_bytes(serialization.Encoding.DER),
                    device_cert.public_bytes(serialization.Encoding.DER),
                )
                print("  Writing private key (unwrapped)...")
                key_data = device_key.private_numbers().private_value.to_bytes(
                    32, "big"
                )
                self.write_private_key(crypto_storage, key_data, wrap=False)
        print("MQTT TLS import OK")

    @CatchExceptions
    def cleanup(self):
        with CryptoStorage(self.get_portname()) as crypto_storage:
            with ReadWriteScope(crypto_storage):
                ret = crypto_storage.wipe_partition(0)
                if ret != 0:
                    raise Exception(f"wipe_partition failed with error {ret}")


if __name__ == "__main__":
    Main()()
