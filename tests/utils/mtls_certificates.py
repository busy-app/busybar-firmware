"""Ephemeral X.509 material for local Fetch mTLS integration tests."""

from __future__ import annotations

import ipaddress
from dataclasses import dataclass
from datetime import UTC, datetime, timedelta

from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import ExtendedKeyUsageOID, NameOID


@dataclass(frozen=True)
class CertificateAuthority:
    certificate: x509.Certificate
    private_key: ec.EllipticCurvePrivateKey

    @property
    def certificate_pem(self) -> bytes:
        return self.certificate.public_bytes(serialization.Encoding.PEM)


@dataclass(frozen=True)
class CertificateKeyPair:
    certificate: x509.Certificate
    private_key: ec.EllipticCurvePrivateKey

    @property
    def certificate_pem(self) -> bytes:
        return self.certificate.public_bytes(serialization.Encoding.PEM)

    @property
    def private_key_pem(self) -> bytes:
        return self.private_key.private_bytes(
            serialization.Encoding.PEM,
            serialization.PrivateFormat.PKCS8,
            serialization.NoEncryption(),
        )

    def encrypted_private_key_pem(self, password: bytes) -> bytes:
        return self.private_key.private_bytes(
            serialization.Encoding.PEM,
            serialization.PrivateFormat.PKCS8,
            serialization.BestAvailableEncryption(password),
        )


def generate_ca(common_name: str) -> CertificateAuthority:
    """Create a short-lived P-256 CA for one pytest session."""
    now = datetime.now(UTC)
    private_key = ec.generate_private_key(ec.SECP256R1())
    subject = x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, common_name)])
    certificate = (
        x509.CertificateBuilder()
        .subject_name(subject)
        .issuer_name(subject)
        .public_key(private_key.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(now - timedelta(days=30))
        .not_valid_after(now + timedelta(days=30))
        .add_extension(x509.BasicConstraints(ca=True, path_length=1), critical=True)
        .add_extension(
            x509.KeyUsage(
                digital_signature=True,
                content_commitment=False,
                key_encipherment=False,
                data_encipherment=False,
                key_agreement=False,
                key_cert_sign=True,
                crl_sign=True,
                encipher_only=False,
                decipher_only=False,
            ),
            critical=True,
        )
        .sign(private_key, hashes.SHA256())
    )
    return CertificateAuthority(certificate, private_key)


def generate_intermediate(
    issuer: CertificateAuthority, common_name: str
) -> CertificateAuthority:
    """Create a P-256 intermediate signed by ``issuer``."""
    now = datetime.now(UTC)
    private_key = ec.generate_private_key(ec.SECP256R1())
    subject = x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, common_name)])
    certificate = (
        x509.CertificateBuilder()
        .subject_name(subject)
        .issuer_name(issuer.certificate.subject)
        .public_key(private_key.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(now - timedelta(days=30))
        .not_valid_after(now + timedelta(days=30))
        .add_extension(x509.BasicConstraints(ca=True, path_length=0), critical=True)
        .add_extension(
            x509.KeyUsage(
                digital_signature=True,
                content_commitment=False,
                key_encipherment=False,
                data_encipherment=False,
                key_agreement=False,
                key_cert_sign=True,
                crl_sign=True,
                encipher_only=False,
                decipher_only=False,
            ),
            critical=True,
        )
        .sign(issuer.private_key, hashes.SHA256())
    )
    return CertificateAuthority(certificate, private_key)


def generate_leaf(
    issuer: CertificateAuthority,
    common_name: str,
    *,
    client_auth: bool,
    server_ip: str | None = None,
    not_valid_before_offset: timedelta = timedelta(minutes=-5),
    not_valid_after_offset: timedelta = timedelta(days=1),
) -> CertificateKeyPair:
    """Create a client- or server-auth leaf signed by ``issuer``."""
    now = datetime.now(UTC)
    private_key = ec.generate_private_key(ec.SECP256R1())
    subject = x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, common_name)])
    eku = (
        ExtendedKeyUsageOID.CLIENT_AUTH
        if client_auth
        else ExtendedKeyUsageOID.SERVER_AUTH
    )
    builder = (
        x509.CertificateBuilder()
        .subject_name(subject)
        .issuer_name(issuer.certificate.subject)
        .public_key(private_key.public_key())
        .serial_number(x509.random_serial_number())
        .not_valid_before(now + not_valid_before_offset)
        .not_valid_after(now + not_valid_after_offset)
        .add_extension(x509.BasicConstraints(ca=False, path_length=None), critical=True)
        .add_extension(
            x509.KeyUsage(
                digital_signature=True,
                content_commitment=False,
                key_encipherment=False,
                data_encipherment=False,
                key_agreement=True,
                key_cert_sign=False,
                crl_sign=False,
                encipher_only=False,
                decipher_only=False,
            ),
            critical=True,
        )
        .add_extension(x509.ExtendedKeyUsage([eku]), critical=False)
    )
    if server_ip is not None:
        builder = builder.add_extension(
            x509.SubjectAlternativeName(
                [x509.IPAddress(ipaddress.ip_address(server_ip))]
            ),
            critical=False,
        )
    certificate = builder.sign(issuer.private_key, hashes.SHA256())
    return CertificateKeyPair(certificate, private_key)
