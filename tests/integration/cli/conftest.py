"""Shared fixtures for the CLI test suite (integration/cli/).

Plain helpers and constants live in utils/cli_helpers.py.
"""

import re
import threading
from dataclasses import dataclass
from datetime import timedelta
from pathlib import Path

import allure
import pytest
from cryptography import x509
from cryptography.x509.oid import NameOID

from clients.api import WIFI_SSID, WifiAPI
from clients.cli import SimpleCLIConnection
from utils.cli_helpers import resync
from utils.fetch_http_server import FetchHTTPServer, FetchRequestHandler
from utils.fetch_mtls_mqtt_broker import FetchMTLSMQTTBroker
from utils.fetch_mtls_server import FetchMTLSServer
from utils.js_test_runner import run_js_case
from utils.mtls_certificates import generate_ca, generate_intermediate, generate_leaf
from utils.wait import wait_for
from utils.wifi_helpers import (
    wait_for_wifi_link_stable,
    wifi_connect_was_already_satisfied,
    wifi_connection_is_active,
)


JS_CLI_LOCAL_STORAGE_PATH = "/ext/apps_data/jsrunner/app.busy.cli.localstorage.json"


@dataclass(frozen=True)
class MTLSMaterial:
    trusted_client_ca_pem: bytes
    valid_client_certificate_pem: bytes
    valid_client_private_key_pem: bytes
    chained_client_certificate_pem: bytes
    chained_client_private_key_pem: bytes
    untrusted_client_certificate_pem: bytes
    untrusted_client_private_key_pem: bytes
    wrong_eku_client_certificate_pem: bytes
    wrong_eku_client_private_key_pem: bytes
    expired_client_certificate_pem: bytes
    expired_client_private_key_pem: bytes
    future_client_certificate_pem: bytes
    future_client_private_key_pem: bytes
    encrypted_client_private_key_pem: bytes
    mismatched_client_private_key_pem: bytes


@dataclass(frozen=True)
class DeviceMTLSIdentity:
    hardware_id: str
    certificate: x509.Certificate
    intermediate: x509.Certificate


def _parse_crypto_certificate(response: str) -> x509.Certificate:
    """Extract a DER certificate from ``crypto read`` CLI output."""
    der = bytearray()
    in_key_data = False
    for raw_line in response.splitlines():
        line = raw_line.strip()
        if line == "key_data:":
            in_key_data = True
            continue
        if not in_key_data:
            continue
        match = re.match(r"^[0-9A-Fa-f]{8}:\s+(.+)$", line)
        if match is None:
            break
        der.extend(bytes.fromhex(match.group(1)))

    assert der, f"certificate data missing from crypto response: {response!r}"
    return x509.load_der_x509_certificate(bytes(der))


@pytest.fixture(scope="session")
def mtls_material() -> MTLSMaterial:
    """Generate all non-production credentials used by local mTLS tests."""
    trusted_ca = generate_ca("Fetch mTLS test root")
    valid_client = generate_leaf(trusted_ca, "Fetch mTLS test client", client_auth=True)

    intermediate = generate_intermediate(trusted_ca, "Fetch mTLS test intermediate")
    chained_client = generate_leaf(
        intermediate, "Fetch mTLS chained client", client_auth=True
    )

    untrusted_ca = generate_ca("Fetch mTLS untrusted root")
    untrusted_client = generate_leaf(
        untrusted_ca, "Fetch mTLS untrusted client", client_auth=True
    )
    wrong_eku_client = generate_leaf(
        trusted_ca, "Fetch mTLS wrong EKU client", client_auth=False
    )
    expired_client = generate_leaf(
        trusted_ca,
        "Fetch mTLS expired client",
        client_auth=True,
        not_valid_before_offset=timedelta(days=-2),
        not_valid_after_offset=timedelta(days=-1),
    )
    future_client = generate_leaf(
        trusted_ca,
        "Fetch mTLS future client",
        client_auth=True,
        not_valid_before_offset=timedelta(days=1),
        not_valid_after_offset=timedelta(days=2),
    )
    mismatched_client = generate_leaf(
        trusted_ca, "Fetch mTLS mismatched client", client_auth=True
    )

    return MTLSMaterial(
        trusted_client_ca_pem=trusted_ca.certificate_pem,
        valid_client_certificate_pem=valid_client.certificate_pem,
        valid_client_private_key_pem=valid_client.private_key_pem,
        chained_client_certificate_pem=(
            chained_client.certificate_pem + intermediate.certificate_pem
        ),
        chained_client_private_key_pem=chained_client.private_key_pem,
        untrusted_client_certificate_pem=untrusted_client.certificate_pem,
        untrusted_client_private_key_pem=untrusted_client.private_key_pem,
        wrong_eku_client_certificate_pem=wrong_eku_client.certificate_pem,
        wrong_eku_client_private_key_pem=wrong_eku_client.private_key_pem,
        expired_client_certificate_pem=expired_client.certificate_pem,
        expired_client_private_key_pem=expired_client.private_key_pem,
        future_client_certificate_pem=future_client.certificate_pem,
        future_client_private_key_pem=future_client.private_key_pem,
        encrypted_client_private_key_pem=valid_client.encrypted_private_key_pem(
            b"fetch-mtls-test"
        ),
        mismatched_client_private_key_pem=mismatched_client.private_key_pem,
    )


@pytest.fixture
def mtls_server_factory(persistent_cli_connection, tmp_path):
    """Start local mTLS servers reachable through the device test network."""
    host_ip = persistent_cli_connection.tn.sock.getsockname()[0]
    server_ca = generate_ca("Fetch mTLS HTTPS server root")
    server_identity = generate_leaf(
        server_ca,
        "Fetch mTLS HTTPS server",
        client_auth=False,
        server_ip=host_ip,
    )
    server_cert_path = Path(tmp_path) / "server.pem"
    server_key_path = Path(tmp_path) / "server.key"
    server_cert_path.write_bytes(server_identity.certificate_pem)
    server_key_path.write_bytes(server_identity.private_key_pem)

    running_servers = []

    def start(
        client_ca_pem: bytes,
        *,
        allow_partial_chain: bool = False,
        tls_version: str | None = None,
    ):
        server = FetchMTLSServer(
            (host_ip, 0),
            server_certificate_path=server_cert_path,
            server_private_key_path=server_key_path,
            client_ca_pem=client_ca_pem.decode(),
            allow_partial_chain=allow_partial_chain,
            tls_version=tls_version,
        )
        server.server_certificate = server_identity.certificate
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        running_servers.append((server, thread))
        return server

    yield start

    for server, thread in running_servers:
        server.release_stall.set()
        server.shutdown()
        thread.join(timeout=2)
        server.server_close()


@pytest.fixture
def mtls_mqtt_broker_factory(persistent_cli_connection, tmp_path):
    """Start local MQTT-over-mTLS brokers reachable by the device."""
    host_ip = persistent_cli_connection.tn.sock.getsockname()[0]
    server_ca = generate_ca("Fetch mTLS MQTT server root")
    server_identity = generate_leaf(
        server_ca,
        "Fetch mTLS MQTT server",
        client_auth=False,
        server_ip=host_ip,
    )
    server_cert_path = Path(tmp_path) / "mqtt-server.pem"
    server_key_path = Path(tmp_path) / "mqtt-server.key"
    server_cert_path.write_bytes(server_identity.certificate_pem)
    server_key_path.write_bytes(server_identity.private_key_pem)

    running_brokers = []

    def start(client_ca_pem: bytes, *, allow_partial_chain: bool = False):
        broker = FetchMTLSMQTTBroker(
            (host_ip, 0),
            server_certificate_path=server_cert_path,
            server_private_key_path=server_key_path,
            client_ca_pem=client_ca_pem.decode(),
            allow_partial_chain=allow_partial_chain,
        )
        thread = threading.Thread(target=broker.serve_forever, daemon=True)
        thread.start()
        running_brokers.append((broker, thread))
        return broker

    yield start

    for broker, thread in running_brokers:
        broker.shutdown()
        thread.join(timeout=2)
        broker.server_close()


@pytest.fixture
def fetch_mtls_account_backend_guard(account_api, mtls_mqtt_broker_factory):
    """Restore MQTT configuration before the local broker is stopped."""
    original_backend = account_api.get_backend()
    try:
        yield
    finally:
        account_api.set_backend(original_backend)


@pytest.fixture
def upload_mtls_credentials(storage_api, storage_dir):
    """Upload ephemeral PEM credentials and let ``storage_dir`` clean them up."""
    uploaded = 0

    def upload(certificate_pem: bytes, private_key_pem: bytes):
        nonlocal uploaded
        uploaded += 1
        certificate_path = f"{storage_dir}/client-{uploaded}.pem"
        private_key_path = f"{storage_dir}/client-{uploaded}.key"

        certificate_response = storage_api.write(certificate_path, certificate_pem)
        key_response = storage_api.write(private_key_path, private_key_pem)
        with allure.step(f"Upload ephemeral mTLS credentials set {uploaded}"):
            assert certificate_response.status_code == 200, (
                "certificate upload failed: "
                f"HTTP {certificate_response.status_code}, "
                f"body={certificate_response.text!r}"
            )
            assert key_response.status_code == 200, (
                f"private-key upload failed: HTTP {key_response.status_code}, "
                f"body={key_response.text!r}"
            )
        return certificate_path, private_key_path

    return upload


@pytest.fixture(scope="module")
def device_mtls_identity(persistent_cli_connection) -> DeviceMTLSIdentity:
    """Read the public device TLS chain without exposing the private key."""
    cli = persistent_cli_connection
    cli.enter_sl_cli()
    try:
        listing = cli.execute_917_command("crypto list 0")
        with allure.step("Verify required device TLS crypto slots"):
            for key_type, key_id in ((12, 0x10), (12, 0x11), (8, 0x11)):
                expected = rf"key:\s+0\s+{key_type}\s+0x{key_id:08X}"
                assert re.search(expected, listing, re.IGNORECASE), (
                    f"required TLS slot {key_type}:0x{key_id:02X} is missing; "
                    f"crypto listing={listing!r}"
                )

        leaf = _parse_crypto_certificate(cli.execute_917_command("crypto read 0 12 11"))
        intermediate = _parse_crypto_certificate(
            cli.execute_917_command("crypto read 0 12 10")
        )
    finally:
        if cli._in_sl_cli:
            cli.exit_sl_cli()

    with allure.step("Validate device certificate identity format"):
        cn = leaf.subject.get_attributes_for_oid(NameOID.COMMON_NAME)
        assert cn, f"device certificate has no common name: {leaf.subject}"
        common_name = str(cn[0].value)
        match = re.fullmatch(r"BusyBar device ([0-9A-Fa-f]{6,64})", common_name)
        assert match, f"unexpected device certificate CN: {common_name!r}"
    return DeviceMTLSIdentity(match.group(1), leaf, intermediate)


@pytest.fixture(scope="module", autouse=True)
def cli_debug(persistent_cli_connection):
    """Debug mode on for the whole CLI module, and left on afterwards.

    The flag lives in NVM and survives reboots, so a suite that turned it off would
    leave the bench without the debug-gated commands (`gpio`, `otp`, `factory_reset`).
    Re-enable on teardown as well: `test_sysctl_debug_toggle` flips it off on purpose.
    """
    persistent_cli_connection.execute_command("sysctl debug 1")
    yield
    persistent_cli_connection.execute_command("sysctl debug 1")


@pytest.fixture(scope="class")
def sl_cli():
    """CLI in 917 (sl_cli) mode, shared by the whole 917 class.

    Its own connection, not `persistent_cli_connection`: `sl_cli` is exclusive and
    a test failing inside 917 mode must not leave the shared CLI at the `917>:`
    prompt. Entering 917 mode is slow, so do it once per class, not per test — the
    commands in there are read-only and cannot interfere with each other.
    """
    cli = SimpleCLIConnection()
    if not cli.connect():
        pytest.skip("Could not connect to CLI")
    try:
        cli.enter_sl_cli()
        yield cli
    finally:
        if cli._in_sl_cli:
            cli.exit_sl_cli()
        cli.disconnect()


@pytest.fixture
def storage_dir(persistent_cli_connection):
    """Empty `/ext/cli_test`, wiped again afterwards — whatever the test did or
    left half-done. Cleanup runs even when the test fails, and starts with a
    resync so it still works if a raw-protocol test died mid-command."""
    cli = persistent_cli_connection
    path = "/ext/cli_test"

    def rm_rf(target):
        # `storage remove` only unlinks files and *empty* dirs, so walk the tree
        # depth-first (`extract` leaves a whole subtree behind in out/)
        listing = cli.execute_command(f"storage list {target}")
        for kind, name in re.findall(r"\[([DF])\]\s+(\S+)", listing):
            child = f"{target}/{name}"
            rm_rf(child) if kind == "D" else cli.execute_command(
                f"storage remove {child}"
            )
        cli.execute_command(f"storage remove {target}")

    rm_rf(path)  # a previous run may have died before its own cleanup
    cli.execute_command(f"storage mkdir {path}")
    try:
        yield path
    finally:
        resync(cli)
        rm_rf(path)


@pytest.fixture
def http_server(persistent_cli_connection):
    """HTTP server on the pytest host, reachable from the device under test."""
    host_ip = persistent_cli_connection.tn.sock.getsockname()[0]
    server = FetchHTTPServer((host_ip, 0), FetchRequestHandler)
    server_thread = threading.Thread(target=server.serve_forever, daemon=True)
    server_thread.start()
    try:
        yield server
    finally:
        server.release_stall.set()
        server.shutdown()
        server_thread.join(timeout=2)
        server.server_close()


@pytest.fixture
def device_wifi_ready(wifi_api: WifiAPI):
    """Ensure the device has a connected Wi-Fi interface and usable IP."""
    with allure.step("Ensure the device is connected to Wi-Fi"):
        initial_status = wifi_api.get_status()
        if initial_status.state == "unknown":
            try:
                initial_status = wait_for(
                    "Wi-Fi service to leave the unknown state",
                    wifi_api.get_status,
                    lambda status: status.state != "unknown",
                    timeout=10,
                    interval=0.5,
                )
            except AssertionError as error:
                pytest.fail(f"Wi-Fi service did not initialize: {error}")

        if not wifi_connection_is_active(initial_status):
            if not WIFI_SSID:
                pytest.skip(
                    "Device Wi-Fi is not ready and WIFI_SSID is not "
                    f"configured; initial status={initial_status!r}"
                )
            response = wifi_api.connect_to_test_network(timeout=30)
            if response.status_code != 200 and not (
                wifi_connect_was_already_satisfied(
                    response.status_code,
                    response.text,
                )
            ):
                pytest.skip(
                    f"Device could not connect to Wi-Fi {WIFI_SSID!r}: "
                    f"HTTP {response.status_code}, {response.text[:200]!r}"
                )
            if response.status_code != 200:
                allure.attach(
                    response.text,
                    name="Wi-Fi connect race accepted",
                    attachment_type=allure.attachment_type.TEXT,
                )

        try:
            ready_status = wait_for_wifi_link_stable(
                wifi_api,
                timeout=45,
            )
        except AssertionError as error:
            pytest.skip(f"Device Wi-Fi did not become ready: {error}")

    return ready_status


@pytest.fixture
def js_case_runner(persistent_cli_connection, storage_api, storage_dir):
    """Upload and run an isolated JavaScript assertion case on the device."""

    def runner(case_name, body, timeout=25):
        return run_js_case(
            persistent_cli_connection,
            storage_api,
            storage_dir,
            case_name,
            body,
            timeout,
        )

    return runner


@pytest.fixture
def js_local_storage_clean(storage_api):
    """Remove the CLI app's persistent localStorage before and after a test."""
    storage_api.remove_raw(JS_CLI_LOCAL_STORAGE_PATH)
    try:
        yield JS_CLI_LOCAL_STORAGE_PATH
    finally:
        storage_api.remove_raw(JS_CLI_LOCAL_STORAGE_PATH)
