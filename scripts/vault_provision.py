#!/usr/bin/env python3
"""
vault_provision.py — Provision BusyBar device TLS credentials via Vault PKI.

Extracts a CSR from the device's secure element, submits it to a HashiCorp
Vault intermediate CA for signing, writes the signed certificate chain
back to the device, and provisions any raw key data stored in Vault's KV
store.

Uses AppRole authentication for machine-identity access to Vault.

Usage:
    python vault_provision.py provision [--vault-addr URL] [--insecure-crypto]
    python vault_provision.py cleanup
    python vault_provision.py upload-key <name> --key-type N --key-id N [--data-hex|--data-file]
    python vault_provision.py list-keys
    python vault_provision.py delete-key <name>
"""

import os
import base64
import re
from pathlib import Path
from datetime import datetime, timezone

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import NameOID, ExtendedKeyUsageOID

from flipper.cli import Cli
from flipper.app import App, CatchExceptions
from crypto_storage import CryptoStorage

try:
    import requests
except ImportError:
    raise SystemExit(
        "This script requires the 'requests' library.\n"
        "Install it with:  pip install requests"
    )


# Key slot constants (must match mqtt_provision.py)
KEY_ID_OFFSET = 0x10
KEY_ID_TLS_SIGN = KEY_ID_OFFSET + 0
KEY_ID_TLS_DEVICE = KEY_ID_OFFSET + 1

KEY_TYPE_ECDSA256_KEY = 8
KEY_TYPE_CSR_DER_ECDSA256 = 11
KEY_TYPE_ECDSA256_CERT = 12

KEY_TYPE_AES256 = 2

# Key types that hold secret material and should be wrapped in secure mode.
# Certificates are public data and are never wrapped.
KEY_TYPES_SECRET = {2, 8}  # AES256=2, EcdsaPriv256=8

# Device-specific AES keys generated on-device (not stored in Vault).
# List of (key_id,) tuples.
DEVICE_AES_KEYS = [
    (0x05, "aes-device-1"),
    (0x06, "aes-device-2"),
]

VAULT_ADDR_DEFAULT = "http://127.0.0.1:8200"
DEVICE_ADDR_DEFAULT = "10.0.4.20:23"

# Default path to pki.conf (in busybar-pki repo, sibling of bsb-firmware)
PKI_CONF_DEFAULT = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    os.pardir,
    os.pardir,
    "busybar-pki",
    "pki.conf",
)


def _load_pki_conf(path: str) -> dict[str, str]:
    """Parse shell-style key=value config file (supports quoted values)."""
    conf = {}
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            m = re.match(r'^([A-Z_]+)="(.*)"$', line)
            if m:
                conf[m.group(1)] = m.group(2)
    return conf


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


class VaultClient:
    """Minimal Vault HTTP client for PKI operations."""

    def __init__(self, addr: str, token: str):
        self.addr = addr.rstrip("/")
        self.token = token
        self.session = requests.Session()
        self.session.headers["X-Vault-Token"] = self.token

    @classmethod
    def from_approle(cls, addr: str, role_id: str, secret_id: str) -> "VaultClient":
        """Authenticate via AppRole and return a VaultClient with a valid token."""
        resp = requests.post(
            f"{addr.rstrip('/')}/v1/auth/approle/login",
            json={"role_id": role_id, "secret_id": secret_id},
            timeout=10,
        )
        resp.raise_for_status()
        token = resp.json()["auth"]["client_token"]
        return cls(addr, token)

    def sign_csr_pem(
        self, csr_pem: str, common_name: str, role: str, ttl: str = "175200h"
    ) -> dict:
        """Submit a CSR to Vault for signing. Returns the response data dict."""
        resp = self.session.post(
            f"{self.addr}/v1/pki_int/sign-verbatim/{role}",
            json={
                "csr": csr_pem,
                "common_name": common_name,
                "ttl": ttl,
                "format": "pem",
            },
            timeout=30,
        )
        resp.raise_for_status()
        return resp.json()["data"]

    def get_ca_chain_pem(self) -> str:
        """Fetch the full CA chain in PEM format."""
        resp = self.session.get(
            f"{self.addr}/v1/pki_int/ca_chain",
            timeout=10,
        )
        resp.raise_for_status()
        return resp.text

    def get_ca_cert_pem(self) -> str:
        """Fetch the intermediate CA certificate in PEM format."""
        resp = self.session.get(
            f"{self.addr}/v1/pki_int/ca/pem",
            timeout=10,
        )
        resp.raise_for_status()
        return resp.text

    # -- KV v2 operations for raw key data --------------------------------

    KV_MOUNT = "secret"
    KV_PREFIX = "busybar-keys"

    def kv_list(self) -> list[str]:
        """List all key names stored in the KV store."""
        resp = self.session.request(
            "LIST",
            f"{self.addr}/v1/{self.KV_MOUNT}/metadata/{self.KV_PREFIX}/",
            timeout=10,
        )
        if resp.status_code == 404:
            return []
        resp.raise_for_status()
        return resp.json()["data"]["keys"]

    def kv_read(self, name: str) -> dict | None:
        """Read a key entry from the KV store. Returns the data dict or None."""
        resp = self.session.get(
            f"{self.addr}/v1/{self.KV_MOUNT}/data/{self.KV_PREFIX}/{name}",
            timeout=10,
        )
        if resp.status_code == 404:
            return None
        resp.raise_for_status()
        return resp.json()["data"]["data"]

    def kv_write(self, name: str, key_type: int, key_id: int, data_hex: str):
        """Write a key entry to the KV store."""
        resp = self.session.post(
            f"{self.addr}/v1/{self.KV_MOUNT}/data/{self.KV_PREFIX}/{name}",
            json={
                "data": {
                    "key_type": key_type,
                    "key_id": key_id,
                    "data": data_hex,
                }
            },
            timeout=10,
        )
        resp.raise_for_status()

    def kv_delete(self, name: str):
        """Delete a key entry from the KV store (metadata + all versions)."""
        resp = self.session.delete(
            f"{self.addr}/v1/{self.KV_MOUNT}/metadata/{self.KV_PREFIX}/{name}",
            timeout=10,
        )
        resp.raise_for_status()


def _make_subject(device_uid: str, conf: dict[str, str]) -> x509.Name:
    cn_template = conf.get("PKI_DEVICE_CN_TEMPLATE", "BusyBar device {uid}")
    return x509.Name(
        [
            x509.NameAttribute(NameOID.COMMON_NAME, cn_template.format(uid=device_uid)),
            x509.NameAttribute(
                NameOID.ORGANIZATION_NAME, conf.get("PKI_ORGANIZATION", "Flipper FZCO")
            ),
            x509.NameAttribute(NameOID.COUNTRY_NAME, conf.get("PKI_COUNTRY", "AE")),
        ]
    )


def _pem_to_der(pem_data: str) -> bytes:
    """Convert the first PEM certificate to DER bytes."""
    cert = x509.load_pem_x509_certificate(pem_data.encode())
    return cert.public_bytes(serialization.Encoding.DER)


def _vault_client_from_env(vault_addr: str) -> VaultClient:
    """Build a VaultClient from environment variables."""
    # Try AppRole first
    role_id = os.environ.get("VAULT_ROLE_ID")
    secret_id = os.environ.get("VAULT_SECRET_ID")
    if role_id and secret_id:
        print("  Authenticating to Vault via AppRole...")
        return VaultClient.from_approle(vault_addr, role_id, secret_id)

    # Fall back to token
    token = os.environ.get("VAULT_TOKEN")
    if token:
        print("  Using VAULT_TOKEN for Vault authentication...")
        return VaultClient(vault_addr, token)

    raise RuntimeError(
        "No Vault credentials found. Set either:\n"
        "  VAULT_ROLE_ID + VAULT_SECRET_ID  (AppRole)\n"
        "  VAULT_TOKEN                       (direct token)"
    )


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        # Provision command
        self.provision_parser = self.subparsers.add_parser(
            "provision", help="Provision device TLS credentials via Vault"
        )
        self.provision_parser.add_argument(
            "--vault-addr",
            type=str,
            default=VAULT_ADDR_DEFAULT,
            help=f"Vault server address (default: {VAULT_ADDR_DEFAULT})",
        )
        self.provision_parser.add_argument(
            "--insecure-crypto",
            action="store_true",
            default=False,
            help="Generate key pair on host (for devices without secure boot)",
        )
        self.provision_parser.add_argument(
            "--pki-conf",
            type=str,
            default=PKI_CONF_DEFAULT,
            help=f"Path to pki.conf (default: {PKI_CONF_DEFAULT})",
        )
        self.provision_parser.add_argument(
            "--device-addr",
            type=str,
            default=DEVICE_ADDR_DEFAULT,
            help=f"Device address as host:port (default: {DEVICE_ADDR_DEFAULT})",
        )
        self.provision_parser.set_defaults(func=self.provision)

        # Cleanup command
        self.cleanup_parser = self.subparsers.add_parser(
            "cleanup", help="Wipe key storage partition"
        )
        self.cleanup_parser.add_argument(
            "--device-addr",
            type=str,
            default=DEVICE_ADDR_DEFAULT,
            help=f"Device address as host:port (default: {DEVICE_ADDR_DEFAULT})",
        )
        self.cleanup_parser.set_defaults(func=self.cleanup)

        # Upload key command (admin)
        self.upload_key_parser = self.subparsers.add_parser(
            "upload-key", help="Upload raw key data to Vault KV store (admin)"
        )
        self.upload_key_parser.add_argument(
            "name", help="Key name (unique label in Vault)"
        )
        self.upload_key_parser.add_argument(
            "--key-type",
            type=lambda x: int(x, 0),
            required=True,
            help="Key type integer (e.g. 8 for ECDSA256_KEY, 12 for ECDSA256_CERT)",
        )
        self.upload_key_parser.add_argument(
            "--key-id",
            type=lambda x: int(x, 0),
            required=True,
            help="Key ID (e.g. 0x12)",
        )
        self.upload_key_parser.add_argument(
            "--data-hex", type=str, default=None, help="Key data as hex string"
        )
        self.upload_key_parser.add_argument(
            "--data-file",
            type=str,
            default=None,
            help="Path to binary file containing key data",
        )
        self.upload_key_parser.add_argument(
            "--vault-addr",
            type=str,
            default=VAULT_ADDR_DEFAULT,
            help=f"Vault server address (default: {VAULT_ADDR_DEFAULT})",
        )
        self.upload_key_parser.set_defaults(func=self.upload_key)

        # List keys command (admin)
        self.list_keys_parser = self.subparsers.add_parser(
            "list-keys", help="List raw keys stored in Vault KV store"
        )
        self.list_keys_parser.add_argument(
            "--vault-addr",
            type=str,
            default=VAULT_ADDR_DEFAULT,
            help=f"Vault server address (default: {VAULT_ADDR_DEFAULT})",
        )
        self.list_keys_parser.set_defaults(func=self.list_keys)

        # Delete key command (admin)
        self.delete_key_parser = self.subparsers.add_parser(
            "delete-key", help="Delete a raw key from Vault KV store (admin)"
        )
        self.delete_key_parser.add_argument("name", help="Key name to delete")
        self.delete_key_parser.add_argument(
            "--vault-addr",
            type=str,
            default=VAULT_ADDR_DEFAULT,
            help=f"Vault server address (default: {VAULT_ADDR_DEFAULT})",
        )
        self.delete_key_parser.set_defaults(func=self.delete_key)

    def get_portname(self):
        addr = getattr(self.args, "device_addr", DEVICE_ADDR_DEFAULT)
        host, _, port = addr.rpartition(":")
        return (host, int(port))

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

    # -- secure path: key never leaves the device ------------------------

    def provision_secure(
        self,
        crypto_storage: CryptoStorage,
        vault: VaultClient,
        device_uid: str,
        conf: dict[str, str],
    ):
        cn_template = conf.get("PKI_DEVICE_CN_TEMPLATE", "BusyBar device {uid}")
        org = conf.get("PKI_ORGANIZATION", "Flipper FZCO")
        country = conf.get("PKI_COUNTRY", "AE")
        role = conf.get("PKI_VAULT_ROLE", "busybar-device")
        ttl = conf.get("PKI_DEVICE_TTL", "175200h")
        cn = cn_template.format(uid=device_uid)
        subject = f"CN={cn},O={org},C={country}"
        print("  Generating wrapped key pair + CSR on device...")
        ret = crypto_storage.gen_csr(0, KEY_ID_TLS_DEVICE, 0, subject, echo=False)
        if ret != 0:
            raise Exception(f"Device CSR generation failed with error {ret}")

        csr_der = crypto_storage.read_key_data(
            0, KEY_TYPE_CSR_DER_ECDSA256, KEY_ID_TLS_DEVICE
        )
        if csr_der is None:
            raise Exception("Failed to read CSR from device")

        # Convert DER CSR to PEM for Vault
        csr = x509.load_der_x509_csr(csr_der)
        csr_pem = csr.public_bytes(serialization.Encoding.PEM).decode()

        print("  Submitting CSR to Vault for signing...")
        result = vault.sign_csr_pem(csr_pem, cn, role, ttl)

        device_cert_pem = result["certificate"]
        ca_chain_pem = result.get("ca_chain", [])
        serial = result.get("serial_number", "unknown")
        print(f"  Certificate issued, serial: {serial}")

        # Get intermediate CA cert for the signing cert slot
        ca_cert_pem = vault.get_ca_cert_pem()

        print("  Writing CA + device certs to device...")
        ca_cert_der = _pem_to_der(ca_cert_pem)
        device_cert_der = _pem_to_der(device_cert_pem)
        self.write_certs(crypto_storage, ca_cert_der, device_cert_der)

    # -- insecure path: key generated on host -----------------------------

    def provision_insecure(
        self,
        crypto_storage: CryptoStorage,
        vault: VaultClient,
        device_uid: str,
        conf: dict[str, str],
    ):
        role = conf.get("PKI_VAULT_ROLE", "busybar-device")
        ttl = conf.get("PKI_DEVICE_TTL", "175200h")
        cn_template = conf.get("PKI_DEVICE_CN_TEMPLATE", "BusyBar device {uid}")
        cn = cn_template.format(uid=device_uid)

        print("  Generating key pair on host...")
        device_private_key = ec.generate_private_key(ec.SECP256R1())

        csr = (
            x509.CertificateSigningRequestBuilder()
            .subject_name(_make_subject(device_uid, conf))
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

        csr_pem = csr.public_bytes(serialization.Encoding.PEM).decode()

        print("  Submitting CSR to Vault for signing...")
        result = vault.sign_csr_pem(csr_pem, cn, role, ttl)

        device_cert_pem = result["certificate"]
        serial = result.get("serial_number", "unknown")
        print(f"  Certificate issued, serial: {serial}")

        ca_cert_pem = vault.get_ca_cert_pem()

        print("  Writing CA + device certs to device...")
        ca_cert_der = _pem_to_der(ca_cert_pem)
        device_cert_der = _pem_to_der(device_cert_pem)
        self.write_certs(crypto_storage, ca_cert_der, device_cert_der)

        print("  Writing private key (unwrapped)...")
        pn = device_private_key.private_numbers()
        key_data = pn.private_value.to_bytes(32, "big")
        self.write_private_key(crypto_storage, key_data, wrap=False)

    # -- raw key provisioning from Vault KV ----------------------------

    def provision_raw_keys(
        self,
        crypto_storage: CryptoStorage,
        vault: VaultClient,
        secure: bool,
    ):
        """Fetch raw keys from Vault KV, write them to the device,
        then generate device-specific AES keys on-device."""
        key_names = vault.kv_list()
        if not key_names:
            print("  No raw keys configured in Vault — skipping.")
        else:
            print(
                f"  Provisioning {len(key_names)} raw key(s) from Vault (secure={secure})..."
            )

            for name in key_names:
                entry = vault.kv_read(name)
                if entry is None:
                    print(
                        f"    Warning: key '{name}' listed but not readable, skipping"
                    )
                    continue

                key_type = int(entry["key_type"])
                key_id = int(entry["key_id"])
                data_hex = entry["data"]
                data_bytes = bytes.fromhex(data_hex)
                data_len = len(data_bytes)

                # Wrap secret material (private keys, symmetric keys) in secure mode;
                # certificates are public and never wrapped.
                wrap = secure and key_type in KEY_TYPES_SECRET
                flags = 1 if wrap else 0

                print(
                    f"    {name}: type={key_type} id=0x{key_id:x} "
                    f"len={data_len} wrap={wrap}"
                )

                ret = crypto_storage.write_key(
                    0,
                    key_type,
                    key_id,
                    flags,
                    data_len,
                    data_hex,
                    echo=False,
                )
                if ret != 0:
                    raise Exception(
                        f"write_key failed for '{name}' (type={key_type}, "
                        f"id=0x{key_id:x}) with error {ret}"
                    )

        # Generate device-specific AES keys directly on the device.
        print(f"  Generating {len(DEVICE_AES_KEYS)} device AES key(s) on-device...")
        for key_id, label in DEVICE_AES_KEYS:
            flags = 1 if secure else 0
            print(
                f"    {label}: gen type={KEY_TYPE_AES256} id=0x{key_id:x} "
                f"wrap={secure}"
            )
            ret = crypto_storage.gen_key(0, KEY_TYPE_AES256, key_id, flags, echo=False)
            if ret != 0:
                raise Exception(
                    f"gen_key failed for '{label}' "
                    f"(id=0x{key_id:x}) with error {ret}"
                )

        vault_count = len(key_names) if key_names else 0
        total = vault_count + len(DEVICE_AES_KEYS)
        print(
            f"  Raw key provisioning complete ({total} key(s): "
            f"{vault_count} from Vault, {len(DEVICE_AES_KEYS)} on-device)."
        )

    # -- commands ---------------------------------------------------------

    @CatchExceptions
    def provision(self):
        vault_addr = self.args.vault_addr
        insecure = self.args.insecure_crypto
        pki_conf_path = self.args.pki_conf

        # Load PKI config
        conf: dict[str, str] = {}
        if os.path.isfile(pki_conf_path):
            conf = _load_pki_conf(pki_conf_path)
            print(f"  Loaded PKI config from {pki_conf_path}")
        else:
            print(f"  Warning: PKI config not found at {pki_conf_path}, using defaults")

        info = _get_device_info(self.get_portname())

        device_uid = info.get("u5_hardware_uid")
        if not device_uid:
            raise RuntimeError("Could not read u5_hardware_uid from device_info")

        secure = not insecure
        if secure and info.get("sl_m4_secureboot") != "true":
            raise RuntimeError(
                "Key wrapping requested but device does not support secure boot"
                " (sl_m4_secureboot is not enabled). Pass --insecure-crypto to"
                " use plain key storage."
            )

        vault = _vault_client_from_env(vault_addr)

        mode = "insecure" if insecure else "secure"
        print(f"Vault TLS provisioning [{mode}] uid={device_uid}")

        with CryptoStorage(self.get_portname()) as crypto_storage:
            print("  Checking TLS slots are empty...")
            self.ensure_tls_slots_empty(crypto_storage)
            if insecure:
                self.provision_insecure(crypto_storage, vault, device_uid, conf)
            else:
                self.provision_secure(crypto_storage, vault, device_uid, conf)

            # Provision raw keys from Vault KV store
            self.provision_raw_keys(crypto_storage, vault, secure=secure)

        print("Vault TLS provisioning OK")

    @CatchExceptions
    def upload_key(self):
        data_hex = self.args.data_hex
        data_file = self.args.data_file

        if not data_hex and not data_file:
            raise RuntimeError("Provide either --data-hex or --data-file")
        if data_hex and data_file:
            raise RuntimeError("Provide only one of --data-hex or --data-file")

        if data_file:
            raw = Path(data_file).read_bytes()
            data_hex = raw.hex()

        # Validate hex
        bytes.fromhex(data_hex)

        vault = _vault_client_from_env(self.args.vault_addr)
        vault.kv_write(
            self.args.name,
            self.args.key_type,
            self.args.key_id,
            data_hex,
        )
        data_len = len(bytes.fromhex(data_hex))
        print(
            f"Uploaded key '{self.args.name}': "
            f"type={self.args.key_type} id=0x{self.args.key_id:x} "
            f"len={data_len}"
        )

    @CatchExceptions
    def list_keys(self):
        vault = _vault_client_from_env(self.args.vault_addr)
        key_names = vault.kv_list()
        if not key_names:
            print("No raw keys stored in Vault.")
            return

        print(f"{'NAME':<30} {'TYPE':>6} {'ID':>6} {'SIZE':>6}")
        print("-" * 52)
        for name in key_names:
            entry = vault.kv_read(name)
            if entry is None:
                print(f"{name:<30} {'?':>6} {'?':>6} {'?':>6}")
                continue
            key_type = int(entry["key_type"])
            key_id = int(entry["key_id"])
            data_len = len(bytes.fromhex(entry["data"]))
            print(f"{name:<30} {key_type:>6} 0x{key_id:>04x} {data_len:>6}")
        print(f"\nTotal: {len(key_names)}")

    @CatchExceptions
    def delete_key(self):
        vault = _vault_client_from_env(self.args.vault_addr)
        vault.kv_delete(self.args.name)
        print(f"Deleted key '{self.args.name}' from Vault.")

    @CatchExceptions
    def cleanup(self):
        with CryptoStorage(self.get_portname()) as crypto_storage:
            ret = crypto_storage.wipe_partition(0)
            if ret != 0:
                raise Exception(f"wipe_partition failed with error {ret}")


if __name__ == "__main__":
    Main()()
