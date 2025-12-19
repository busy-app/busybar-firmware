#!/usr/bin/env python3

import argparse
import hashlib
import struct
import sys
import time
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from typing import Optional

# Optional cryptography import for signature operations
try:
    from cryptography.hazmat.primitives import hashes, serialization
    from cryptography.hazmat.primitives.asymmetric import ec
    from cryptography.hazmat.primitives.asymmetric.utils import (
        decode_dss_signature,
        encode_dss_signature,
    )

    HAS_CRYPTO = True
except ImportError:
    HAS_CRYPTO = False

OTP_PAGE_SIZE = 128


def pad_to_page(data: bytes) -> bytes:
    """Pad data to OTP page size (128 bytes)"""
    if len(data) > OTP_PAGE_SIZE:
        raise ValueError(f"Data too large for OTP page: {len(data)} > {OTP_PAGE_SIZE}")
    return data.ljust(OTP_PAGE_SIZE, b"\x00")


def format_mac(mac_bytes: bytes) -> str:
    """Format MAC address bytes as colon-separated hex string"""
    return ":".join(f"{b:02x}" for b in mac_bytes)


def parse_mac(mac_str: str) -> bytes:
    """Parse MAC address from colon or dash separated hex string"""
    mac_str = mac_str.replace("-", ":").replace(" ", ":")
    parts = mac_str.split(":")
    if len(parts) != 6:
        raise ValueError(f"Invalid MAC address format: {mac_str}")
    return bytes(int(p, 16) for p in parts)


def format_hex(data: bytes) -> str:
    """Format bytes as hex string"""
    return data.hex()


def parse_hex(hex_str: str) -> bytes:
    """Parse hex string to bytes"""
    return bytes.fromhex(hex_str.replace(" ", "").replace(":", ""))


@dataclass
class OTP1Data:
    """
    hw_otp1_ver                  : 0                  # uint8
    hw_timestamp                 : 1751035273         # uint32
    u5_usb_mac                   : aa:cc:33:44:55:66  # uint8[6]
    hw_model                     : BB.1               # str(8)
    hw_version                   : 4                  # uint8
    hw_target                    : 22                 # uint8
    hw_body                      : 7                  # uint8
    hw_connect                   : 2                  # uint8
    """

    hw_otp1_ver: int
    hw_timestamp: int
    u5_usb_mac: bytes
    hw_model: str
    hw_version: int
    hw_target: int
    hw_body: int
    hw_connect: int

    STRUCT_FORMAT = "<B I 6s 8s B B B B"

    @classmethod
    def from_bytes(cls, data: bytes) -> "OTP1Data":
        unpacked_data = struct.unpack(cls.STRUCT_FORMAT, data)
        return cls(
            hw_otp1_ver=unpacked_data[0],
            hw_timestamp=unpacked_data[1],
            u5_usb_mac=unpacked_data[2],
            hw_model=unpacked_data[3].decode("utf-8").rstrip("\x00"),
            hw_version=unpacked_data[4],
            hw_target=unpacked_data[5],
            hw_body=unpacked_data[6],
            hw_connect=unpacked_data[7],
        )

    def to_bytes(self) -> bytes:
        return struct.pack(
            self.STRUCT_FORMAT,
            self.hw_otp1_ver,
            self.hw_timestamp,
            self.u5_usb_mac,
            self.hw_model.encode("utf-8").ljust(8, b"\x00"),
            self.hw_version,
            self.hw_target,
            self.hw_body,
            self.hw_connect,
        )


class HWColor(Enum):
    WHITE = 0x00


class HWRegion(Enum):
    WORLD = 0x00


@dataclass
class OTP2Data:
    """
    hw_otp2_ver                  : 0                  # uint8
    hw_timestamp_qc              : 1751035273         # uint32
    hw_color                     : 0                  # uint8
    hw_region                    : 0                  # uint8
    """

    STRUCT_FORMAT = "<B I B B"

    hw_otp2_ver: int
    hw_timestamp_qc: int
    hw_color: HWColor
    hw_region: HWRegion

    @classmethod
    def from_bytes(cls, data: bytes) -> "OTP2Data":
        unpacked_data = struct.unpack(cls.STRUCT_FORMAT, data)
        return cls(
            hw_otp2_ver=unpacked_data[0],
            hw_timestamp_qc=unpacked_data[1],
            hw_color=HWColor(unpacked_data[2]),
            hw_region=HWRegion(unpacked_data[3]),
        )

    def to_bytes(self) -> bytes:
        return struct.pack(
            self.STRUCT_FORMAT,
            self.hw_otp2_ver,
            self.hw_timestamp_qc,
            self.hw_color.value,
            self.hw_region.value,
        )


class ECCurve(Enum):
    SECP224R1 = 1


@dataclass
class OTP3Data:
    """
    hw_otp3_ver                  : 0                  # uint8
    hw_otp3_curve                : 1                  # uint8
    hw_otp3_pkey                 : <public key>      # uint8[56]
    """

    STRUCT_FORMAT = "<B B 56s"

    hw_otp3_ver: int
    hw_otp3_curve: ECCurve
    hw_otp3_pkey: bytes

    @classmethod
    def from_bytes(cls, data: bytes) -> "OTP3Data":
        unpacked_data = struct.unpack(cls.STRUCT_FORMAT, data)
        return cls(
            hw_otp3_ver=unpacked_data[0],
            hw_otp3_curve=ECCurve(unpacked_data[1]),
            hw_otp3_pkey=unpacked_data[2],
        )

    def to_bytes(self) -> bytes:
        return struct.pack(
            self.STRUCT_FORMAT,
            self.hw_otp3_ver,
            self.hw_otp3_curve.value,
            self.hw_otp3_pkey,
        )


@dataclass
class OTPSignature:
    """
    hw_otp4_ver                : 0                  # uint8
    hw_otp4_mcu_uid            : <mcu id>          # uint8[12]
    hw_otp1_signature          : <signature>       # uint8[56]
    hw_otp2_signature          : <signature>       # uint8[56]
    """

    STRUCT_FORMAT = "<B 12s 56s 56s"

    hw_otp4_ver: int
    hw_otp4_mcu_uid: bytes
    hw_otp1_signature: bytes
    hw_otp2_signature: bytes

    @classmethod
    def from_bytes(cls, data: bytes) -> "OTPSignature":
        unpacked_data = struct.unpack(cls.STRUCT_FORMAT, data)
        return cls(
            hw_otp4_ver=unpacked_data[0],
            hw_otp4_mcu_uid=unpacked_data[1],
            hw_otp1_signature=unpacked_data[2],
            hw_otp2_signature=unpacked_data[3],
        )

    def to_bytes(self) -> bytes:
        return struct.pack(
            self.STRUCT_FORMAT,
            self.hw_otp4_ver,
            self.hw_otp4_mcu_uid,
            self.hw_otp1_signature,
            self.hw_otp2_signature,
        )


# ============================================================================
# Signature and Key Management Functions
# ============================================================================


def require_crypto():
    """Raise error if cryptography module is not available"""
    if not HAS_CRYPTO:
        raise ImportError(
            "cryptography module required for signature operations. "
            "Install with: pip install cryptography"
        )


def generate_keypair() -> tuple[bytes, bytes]:
    """Generate a new ECDSA secp224r1 keypair.

    Returns:
        Tuple of (private_key_bytes, public_key_bytes)
        Private key is 28 bytes, public key is 56 bytes (uncompressed x||y)
    """
    require_crypto()
    private_key = ec.generate_private_key(ec.SECP224R1())
    public_key = private_key.public_key()

    # Extract raw private key bytes (28 bytes for secp224r1)
    private_numbers = private_key.private_numbers()
    private_bytes = private_numbers.private_value.to_bytes(28, byteorder="big")

    # Extract raw public key bytes (56 bytes: x || y, each 28 bytes)
    public_numbers = public_key.public_numbers()
    public_bytes = public_numbers.x.to_bytes(
        28, byteorder="big"
    ) + public_numbers.y.to_bytes(28, byteorder="big")

    return private_bytes, public_bytes


def load_private_key(key_path: Path) -> "ec.EllipticCurvePrivateKey":
    """Load private key from PEM file"""
    require_crypto()
    with open(key_path, "rb") as f:
        return serialization.load_pem_private_key(f.read(), password=None)


def save_private_key(private_key: "ec.EllipticCurvePrivateKey", key_path: Path):
    """Save private key to PEM file"""
    require_crypto()
    pem = private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.PKCS8,
        encryption_algorithm=serialization.NoEncryption(),
    )
    with open(key_path, "wb") as f:
        f.write(pem)


def private_key_from_bytes(private_bytes: bytes) -> "ec.EllipticCurvePrivateKey":
    """Reconstruct private key from raw bytes"""
    require_crypto()
    private_value = int.from_bytes(private_bytes, byteorder="big")
    # Generate a temporary key to get the curve parameters
    private_key = ec.derive_private_key(private_value, ec.SECP224R1())
    return private_key


def public_key_from_bytes(public_bytes: bytes) -> "ec.EllipticCurvePublicKey":
    """Reconstruct public key from raw 56-byte x||y format"""
    require_crypto()
    if len(public_bytes) != 56:
        raise ValueError(f"Public key must be 56 bytes, got {len(public_bytes)}")
    x = int.from_bytes(public_bytes[:28], byteorder="big")
    y = int.from_bytes(public_bytes[28:], byteorder="big")
    public_numbers = ec.EllipticCurvePublicNumbers(x, y, ec.SECP224R1())
    return public_numbers.public_key()


def sign_data(private_key: "ec.EllipticCurvePrivateKey", data: bytes) -> bytes:
    """Sign data with ECDSA secp224r1 and return fixed-size signature.

    Returns:
        56-byte signature (r || s, each 28 bytes)
    """
    require_crypto()
    # Sign with SHA256
    signature = private_key.sign(data, ec.ECDSA(hashes.SHA256()))

    # Decode DER signature to get r and s values
    r, s = decode_dss_signature(signature)

    # Convert to fixed-size format (28 bytes each for secp224r1)
    r_bytes = r.to_bytes(28, byteorder="big")
    s_bytes = s.to_bytes(28, byteorder="big")

    return r_bytes + s_bytes


def verify_signature(
    public_key: "ec.EllipticCurvePublicKey", data: bytes, signature: bytes
) -> bool:
    """Verify ECDSA signature.

    Args:
        public_key: EC public key object
        data: Original data that was signed
        signature: 56-byte signature (r || s format)

    Returns:
        True if signature is valid, False otherwise
    """
    require_crypto()
    if len(signature) != 56:
        return False

    try:
        # Convert fixed-size signature back to DER format
        r = int.from_bytes(signature[:28], byteorder="big")
        s = int.from_bytes(signature[28:], byteorder="big")
        der_signature = encode_dss_signature(r, s)

        # Verify
        public_key.verify(der_signature, data, ec.ECDSA(hashes.SHA256()))
        return True
    except Exception:
        return False


def create_otp1_signature_data(otp1: OTP1Data, mcu_uid: bytes) -> bytes:
    """Create data to be signed for OTP1 (includes MCU UID for binding)"""
    return mcu_uid + otp1.to_bytes()


def create_otp2_signature_data(otp2: OTP2Data, mcu_uid: bytes) -> bytes:
    """Create data to be signed for OTP2 (includes MCU UID for binding)"""
    return mcu_uid + otp2.to_bytes()


# ============================================================================
# Command-line Interface
# ============================================================================


def cmd_generate_key(args):
    """Generate a new ECDSA keypair"""
    require_crypto()
    private_key = ec.generate_private_key(ec.SECP224R1())
    save_private_key(private_key, Path(args.output))
    print(f"Private key saved to: {args.output}")

    # Also output public key if requested
    if args.public:
        public_key = private_key.public_key()
        public_numbers = public_key.public_numbers()
        public_bytes = public_numbers.x.to_bytes(
            28, byteorder="big"
        ) + public_numbers.y.to_bytes(28, byteorder="big")
        with open(args.public, "wb") as f:
            f.write(public_bytes)
        print(f"Public key (raw) saved to: {args.public}")
        print(f"Public key hex: {public_bytes.hex()}")


def cmd_create_otp1(args):
    """Create OTP1 data file"""
    otp1 = OTP1Data(
        hw_otp1_ver=args.version,
        hw_timestamp=args.timestamp if args.timestamp else int(time.time()),
        u5_usb_mac=parse_mac(args.mac),
        hw_model=args.model,
        hw_version=args.hw_version,
        hw_target=args.target,
        hw_body=args.body,
        hw_connect=args.connect,
    )

    data = pad_to_page(otp1.to_bytes())
    with open(args.output, "wb") as f:
        f.write(data)

    print(f"OTP1 data saved to: {args.output}")
    print(f"  Version: {otp1.hw_otp1_ver}")
    print(f"  Timestamp: {otp1.hw_timestamp}")
    print(f"  MAC: {format_mac(otp1.u5_usb_mac)}")
    print(f"  Model: {otp1.hw_model}")
    print(f"  HW Version: {otp1.hw_version}")
    print(f"  Target: {otp1.hw_target}")
    print(f"  Body: {otp1.hw_body}")
    print(f"  Connect: {otp1.hw_connect}")


def cmd_create_otp2(args):
    """Create OTP2 data file"""
    otp2 = OTP2Data(
        hw_otp2_ver=args.version,
        hw_timestamp_qc=args.timestamp if args.timestamp else int(time.time()),
        hw_color=HWColor(args.color),
        hw_region=HWRegion(args.region),
    )

    data = pad_to_page(otp2.to_bytes())
    with open(args.output, "wb") as f:
        f.write(data)

    print(f"OTP2 data saved to: {args.output}")
    print(f"  Version: {otp2.hw_otp2_ver}")
    print(f"  QC Timestamp: {otp2.hw_timestamp_qc}")
    print(f"  Color: {otp2.hw_color.name}")
    print(f"  Region: {otp2.hw_region.name}")


def cmd_create_otp3(args):
    """Create OTP3 data file (public key storage)"""
    require_crypto()

    # Load private key and extract public key
    private_key = load_private_key(Path(args.key))
    public_key = private_key.public_key()
    public_numbers = public_key.public_numbers()
    public_bytes = public_numbers.x.to_bytes(
        28, byteorder="big"
    ) + public_numbers.y.to_bytes(28, byteorder="big")

    otp3 = OTP3Data(
        hw_otp3_ver=args.version,
        hw_otp3_curve=ECCurve.SECP224R1,
        hw_otp3_pkey=public_bytes,
    )

    data = pad_to_page(otp3.to_bytes())
    with open(args.output, "wb") as f:
        f.write(data)

    print(f"OTP3 data saved to: {args.output}")
    print(f"  Version: {otp3.hw_otp3_ver}")
    print(f"  Curve: {otp3.hw_otp3_curve.name}")
    print(f"  Public Key: {public_bytes.hex()}")


def cmd_create_otp4(args):
    """Create OTP4 signature file"""
    require_crypto()

    # Load private key
    private_key = load_private_key(Path(args.key))

    # Load OTP1 and OTP2 data
    with open(args.otp1, "rb") as f:
        otp1_raw = f.read()
    otp1 = OTP1Data.from_bytes(otp1_raw[: struct.calcsize(OTP1Data.STRUCT_FORMAT)])

    with open(args.otp2, "rb") as f:
        otp2_raw = f.read()
    otp2 = OTP2Data.from_bytes(otp2_raw[: struct.calcsize(OTP2Data.STRUCT_FORMAT)])

    # Parse MCU UID
    mcu_uid = parse_hex(args.mcu_uid)
    if len(mcu_uid) != 12:
        raise ValueError(f"MCU UID must be 12 bytes, got {len(mcu_uid)}")

    # Create signatures
    otp1_sig_data = create_otp1_signature_data(otp1, mcu_uid)
    otp2_sig_data = create_otp2_signature_data(otp2, mcu_uid)

    otp1_signature = sign_data(private_key, otp1_sig_data)
    otp2_signature = sign_data(private_key, otp2_sig_data)

    otp4 = OTPSignature(
        hw_otp4_ver=args.version,
        hw_otp4_mcu_uid=mcu_uid,
        hw_otp1_signature=otp1_signature,
        hw_otp2_signature=otp2_signature,
    )

    data = pad_to_page(otp4.to_bytes())
    with open(args.output, "wb") as f:
        f.write(data)

    print(f"OTP4 signature saved to: {args.output}")
    print(f"  Version: {otp4.hw_otp4_ver}")
    print(f"  MCU UID: {format_hex(mcu_uid)}")
    print(f"  OTP1 Signature: {format_hex(otp1_signature)}")
    print(f"  OTP2 Signature: {format_hex(otp2_signature)}")


def cmd_load(args):
    """Load and display OTP file contents"""
    with open(args.file, "rb") as f:
        data = f.read()

    if len(data) < OTP_PAGE_SIZE:
        print(
            f"Warning: File is smaller than OTP page size ({len(data)} < {OTP_PAGE_SIZE})"
        )

    otp_type = args.type.upper()

    if otp_type == "OTP1":
        otp = OTP1Data.from_bytes(data[: struct.calcsize(OTP1Data.STRUCT_FORMAT)])
        print("OTP1 Data:")
        print(f"  hw_otp1_ver    : {otp.hw_otp1_ver}")
        print(f"  hw_timestamp   : {otp.hw_timestamp}")
        print(f"  u5_usb_mac     : {format_mac(otp.u5_usb_mac)}")
        print(f"  hw_model       : {otp.hw_model}")
        print(f"  hw_version     : {otp.hw_version}")
        print(f"  hw_target      : {otp.hw_target}")
        print(f"  hw_body        : {otp.hw_body}")
        print(f"  hw_connect     : {otp.hw_connect}")

    elif otp_type == "OTP2":
        otp = OTP2Data.from_bytes(data[: struct.calcsize(OTP2Data.STRUCT_FORMAT)])
        print("OTP2 Data:")
        print(f"  hw_otp2_ver      : {otp.hw_otp2_ver}")
        print(f"  hw_timestamp_qc  : {otp.hw_timestamp_qc}")
        print(f"  hw_color         : {otp.hw_color.name} ({otp.hw_color.value})")
        print(f"  hw_region        : {otp.hw_region.name} ({otp.hw_region.value})")

    elif otp_type == "OTP3":
        otp = OTP3Data.from_bytes(data[: struct.calcsize(OTP3Data.STRUCT_FORMAT)])
        print("OTP3 Data:")
        print(f"  hw_otp3_ver   : {otp.hw_otp3_ver}")
        print(f"  hw_otp3_curve : {otp.hw_otp3_curve.name} ({otp.hw_otp3_curve.value})")
        print(f"  hw_otp3_pkey  : {format_hex(otp.hw_otp3_pkey)}")

    elif otp_type == "OTP4":
        otp = OTPSignature.from_bytes(
            data[: struct.calcsize(OTPSignature.STRUCT_FORMAT)]
        )
        print("OTP4 Signature Data:")
        print(f"  hw_otp4_ver       : {otp.hw_otp4_ver}")
        print(f"  hw_otp4_mcu_uid   : {format_hex(otp.hw_otp4_mcu_uid)}")
        print(f"  hw_otp1_signature : {format_hex(otp.hw_otp1_signature)}")
        print(f"  hw_otp2_signature : {format_hex(otp.hw_otp2_signature)}")

    else:
        print(f"Unknown OTP type: {otp_type}")
        sys.exit(1)


def cmd_verify(args):
    """Verify OTP signatures"""
    require_crypto()

    # Load OTP3 (public key)
    with open(args.otp3, "rb") as f:
        otp3_raw = f.read()
    otp3 = OTP3Data.from_bytes(otp3_raw[: struct.calcsize(OTP3Data.STRUCT_FORMAT)])
    public_key = public_key_from_bytes(otp3.hw_otp3_pkey)

    # Load OTP4 (signatures)
    with open(args.otp4, "rb") as f:
        otp4_raw = f.read()
    otp4 = OTPSignature.from_bytes(
        otp4_raw[: struct.calcsize(OTPSignature.STRUCT_FORMAT)]
    )

    # Load OTP1 and OTP2
    with open(args.otp1, "rb") as f:
        otp1_raw = f.read()
    otp1 = OTP1Data.from_bytes(otp1_raw[: struct.calcsize(OTP1Data.STRUCT_FORMAT)])

    with open(args.otp2, "rb") as f:
        otp2_raw = f.read()
    otp2 = OTP2Data.from_bytes(otp2_raw[: struct.calcsize(OTP2Data.STRUCT_FORMAT)])

    # Verify signatures
    otp1_sig_data = create_otp1_signature_data(otp1, otp4.hw_otp4_mcu_uid)
    otp2_sig_data = create_otp2_signature_data(otp2, otp4.hw_otp4_mcu_uid)

    otp1_valid = verify_signature(public_key, otp1_sig_data, otp4.hw_otp1_signature)
    otp2_valid = verify_signature(public_key, otp2_sig_data, otp4.hw_otp2_signature)

    print(f"OTP1 signature: {'VALID' if otp1_valid else 'INVALID'}")
    print(f"OTP2 signature: {'VALID' if otp2_valid else 'INVALID'}")

    if not (otp1_valid and otp2_valid):
        sys.exit(1)
    print("\nAll signatures verified successfully!")


def cmd_dump_all(args):
    """Dump all OTP pages from a combined binary file"""
    with open(args.file, "rb") as f:
        data = f.read()

    offset = args.offset
    print(f"Reading OTP data from offset 0x{offset:x}")

    for i, (name, cls) in enumerate(
        [
            ("OTP1", OTP1Data),
            ("OTP2", OTP2Data),
            ("OTP3", OTP3Data),
            ("OTP4", OTPSignature),
        ]
    ):
        page_offset = offset + i * OTP_PAGE_SIZE
        if page_offset + OTP_PAGE_SIZE > len(data):
            print(f"\n{name}: Not enough data")
            continue

        page_data = data[page_offset : page_offset + OTP_PAGE_SIZE]
        struct_size = struct.calcsize(cls.STRUCT_FORMAT)

        print(f"\n{name} (offset 0x{page_offset:x}):")
        try:
            otp = cls.from_bytes(page_data[:struct_size])
            if name == "OTP1":
                print(f"  hw_otp1_ver    : {otp.hw_otp1_ver}")
                print(f"  hw_timestamp   : {otp.hw_timestamp}")
                print(f"  u5_usb_mac     : {format_mac(otp.u5_usb_mac)}")
                print(f"  hw_model       : {otp.hw_model}")
                print(f"  hw_version     : {otp.hw_version}")
                print(f"  hw_target      : {otp.hw_target}")
                print(f"  hw_body        : {otp.hw_body}")
                print(f"  hw_connect     : {otp.hw_connect}")
            elif name == "OTP2":
                print(f"  hw_otp2_ver      : {otp.hw_otp2_ver}")
                print(f"  hw_timestamp_qc  : {otp.hw_timestamp_qc}")
                print(f"  hw_color         : {otp.hw_color.name}")
                print(f"  hw_region        : {otp.hw_region.name}")
            elif name == "OTP3":
                print(f"  hw_otp3_ver   : {otp.hw_otp3_ver}")
                print(f"  hw_otp3_curve : {otp.hw_otp3_curve.name}")
                print(f"  hw_otp3_pkey  : {format_hex(otp.hw_otp3_pkey)}")
            elif name == "OTP4":
                print(f"  hw_otp4_ver       : {otp.hw_otp4_ver}")
                print(f"  hw_otp4_mcu_uid   : {format_hex(otp.hw_otp4_mcu_uid)}")
                print(f"  hw_otp1_signature : {format_hex(otp.hw_otp1_signature)}")
                print(f"  hw_otp2_signature : {format_hex(otp.hw_otp2_signature)}")
        except Exception as e:
            print(f"  Error parsing: {e}")


def main():
    parser = argparse.ArgumentParser(
        description="STM32U5 OTP Page Management Tool",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Generate signing keypair
  %(prog)s genkey -o private.pem --public public.bin

  # Create OTP1 (hardware info)
  %(prog)s create-otp1 -o otp1.bin --mac aa:bb:cc:dd:ee:ff --model "BB.1" \\
      --hw-version 4 --target 22 --body 7 --connect 2

  # Create OTP2 (QC info)
  %(prog)s create-otp2 -o otp2.bin --color 0 --region 0

  # Create OTP3 (public key)
  %(prog)s create-otp3 -o otp3.bin --key private.pem

  # Create OTP4 (signatures)
  %(prog)s create-otp4 -o otp4.bin --key private.pem --otp1 otp1.bin \\
      --otp2 otp2.bin --mcu-uid 001122334455667788990011

  # Load and display OTP file
  %(prog)s load otp1.bin --type otp1

  # Verify signatures
  %(prog)s verify --otp1 otp1.bin --otp2 otp2.bin --otp3 otp3.bin --otp4 otp4.bin
""",
    )

    subparsers = parser.add_subparsers(dest="command", help="Commands")

    # Generate key command
    genkey_parser = subparsers.add_parser("genkey", help="Generate ECDSA keypair")
    genkey_parser.add_argument(
        "-o", "--output", required=True, help="Output private key file (PEM)"
    )
    genkey_parser.add_argument("--public", help="Output public key file (raw binary)")
    genkey_parser.set_defaults(func=cmd_generate_key)

    # Create OTP1 command
    otp1_parser = subparsers.add_parser("create-otp1", help="Create OTP1 hardware info")
    otp1_parser.add_argument("-o", "--output", required=True, help="Output file")
    otp1_parser.add_argument(
        "--version", type=int, default=0, help="OTP1 version (default: 0)"
    )
    otp1_parser.add_argument(
        "--timestamp", type=int, help="Unix timestamp (default: now)"
    )
    otp1_parser.add_argument(
        "--mac", required=True, help="USB MAC address (aa:bb:cc:dd:ee:ff)"
    )
    otp1_parser.add_argument(
        "--model", required=True, help="Hardware model (max 8 chars)"
    )
    otp1_parser.add_argument(
        "--hw-version", type=int, required=True, help="Hardware version"
    )
    otp1_parser.add_argument("--target", type=int, required=True, help="Target ID")
    otp1_parser.add_argument("--body", type=int, required=True, help="Body type")
    otp1_parser.add_argument("--connect", type=int, required=True, help="Connect type")
    otp1_parser.set_defaults(func=cmd_create_otp1)

    # Create OTP2 command
    otp2_parser = subparsers.add_parser("create-otp2", help="Create OTP2 QC info")
    otp2_parser.add_argument("-o", "--output", required=True, help="Output file")
    otp2_parser.add_argument(
        "--version", type=int, default=0, help="OTP2 version (default: 0)"
    )
    otp2_parser.add_argument(
        "--timestamp", type=int, help="QC timestamp (default: now)"
    )
    otp2_parser.add_argument(
        "--color", type=int, default=0, help="Hardware color (default: 0=WHITE)"
    )
    otp2_parser.add_argument(
        "--region", type=int, default=0, help="Region (default: 0=WORLD)"
    )
    otp2_parser.set_defaults(func=cmd_create_otp2)

    # Create OTP3 command
    otp3_parser = subparsers.add_parser("create-otp3", help="Create OTP3 public key")
    otp3_parser.add_argument("-o", "--output", required=True, help="Output file")
    otp3_parser.add_argument(
        "--version", type=int, default=0, help="OTP3 version (default: 0)"
    )
    otp3_parser.add_argument("--key", required=True, help="Private key file (PEM)")
    otp3_parser.set_defaults(func=cmd_create_otp3)

    # Create OTP4 command
    otp4_parser = subparsers.add_parser("create-otp4", help="Create OTP4 signatures")
    otp4_parser.add_argument("-o", "--output", required=True, help="Output file")
    otp4_parser.add_argument(
        "--version", type=int, default=0, help="OTP4 version (default: 0)"
    )
    otp4_parser.add_argument("--key", required=True, help="Private key file (PEM)")
    otp4_parser.add_argument("--otp1", required=True, help="OTP1 data file")
    otp4_parser.add_argument("--otp2", required=True, help="OTP2 data file")
    otp4_parser.add_argument("--mcu-uid", required=True, help="MCU UID (12 bytes hex)")
    otp4_parser.set_defaults(func=cmd_create_otp4)

    # Load command
    load_parser = subparsers.add_parser("load", help="Load and display OTP file")
    load_parser.add_argument("file", help="OTP file to load")
    load_parser.add_argument(
        "--type",
        "-t",
        required=True,
        choices=["otp1", "otp2", "otp3", "otp4"],
        help="OTP type",
    )
    load_parser.set_defaults(func=cmd_load)

    # Verify command
    verify_parser = subparsers.add_parser("verify", help="Verify OTP signatures")
    verify_parser.add_argument("--otp1", required=True, help="OTP1 data file")
    verify_parser.add_argument("--otp2", required=True, help="OTP2 data file")
    verify_parser.add_argument("--otp3", required=True, help="OTP3 public key file")
    verify_parser.add_argument("--otp4", required=True, help="OTP4 signature file")
    verify_parser.set_defaults(func=cmd_verify)

    # Dump all command
    dump_parser = subparsers.add_parser("dump", help="Dump all OTP pages from binary")
    dump_parser.add_argument("file", help="Binary file containing OTP data")
    dump_parser.add_argument(
        "--offset",
        type=lambda x: int(x, 0),
        default=0,
        help="Offset to OTP data (default: 0)",
    )
    dump_parser.set_defaults(func=cmd_dump_all)

    args = parser.parse_args()

    if not args.command:
        parser.print_help()
        sys.exit(1)

    try:
        args.func(args)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
