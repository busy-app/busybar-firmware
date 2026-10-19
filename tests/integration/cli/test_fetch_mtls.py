"""Mutual TLS coverage for the device-side ``fetch`` CLI command."""

import queue
import re

import allure
import pytest
from cryptography import x509
from cryptography.hazmat.primitives import serialization
from cryptography.x509.oid import NameOID

from clients.api import AccountBackend
from utils.wait import wait_for


pytestmark = [pytest.mark.cli, pytest.mark.mtls]

CLOUD_WEATHER_URL = (
    "https://api.dev.busy.app/weather/v1/forecast?latitude=1&longitude=1"
)
PUBLIC_CA_PROBE_URL = "https://update.busy.app/busybar-firmware/directory.json"
MQTT_CONNECTION_TIMEOUT_SECONDS = 70  # Firmware reconnect backoff caps at 60 s.


def _get_captured_request(server, response: str) -> dict:
    with allure.step("Read the request captured by the local mTLS server"):
        try:
            return server.requests.get(timeout=1)
        except queue.Empty:
            pytest.fail(f"mTLS server captured no request; Fetch output={response!r}")


def _assert_tls_rejected(cli, response: str, server) -> None:
    with allure.step("Verify TLS rejection produced no authenticated HTTP request"):
        assert "HTTP/" not in response, f"unexpected HTTP response: {response!r}"
        assert server.requests.empty(), (
            "server received authenticated HTTP requests; "
            f"queue size={server.requests.qsize()}, Fetch output={response!r}"
        )

    with allure.step("Verify Fetch returned control to the CLI"):
        marker = "__fetch_mtls_rejection_complete__"
        probe = cli.execute_command(f"echo {marker}", timeout=5)
        assert marker in probe, (
            "Fetch did not return control to CLI after TLS rejection; "
            f"Fetch output={response!r}, probe output={probe!r}"
        )


@allure.epic("BSB CLI Testing")
@allure.feature("Fetch mTLS")
class TestFetchTLSArguments:
    @allure.title("CLI. Fetch usage documents all mTLS options exactly once.")
    def test_help_documents_mtls_options(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("fetch")
        expected_options = [
            "-k (TLS) Ignore server certificate",
            '-a (TLS) Client auth type ("none" (default), "device" or "cert")',
            "-C (TLS) Custom client certificate file path",
            "-K (TLS) Custom client private key file path",
        ]

        with allure.step("Verify mTLS option text, ordering, and uniqueness"):
            positions = []
            for option in expected_options:
                assert response.count(option) == 1, (
                    f"expected one exact {option!r} line in Fetch usage, got "
                    f"{response!r}"
                )
                positions.append(response.index(option))
            assert positions == sorted(
                positions
            ), f"mTLS options are in an unexpected order: {response!r}"

    @allure.title("CLI. Fetch rejects an unknown TLS client authentication type.")
    def test_invalid_client_auth_type(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command(
            "fetch -a invalid https://example.invalid"
        )
        with allure.step("Verify invalid client authentication type diagnostics"):
            assert "Error: Invalid client auth type" in response, response

    @allure.title("CLI. Fetch requires a certificate path for custom mTLS.")
    def test_custom_auth_requires_certificate(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command(
            "fetch -a cert -K /ext/missing.key https://example.invalid"
        )
        with allure.step("Verify missing certificate-path diagnostics"):
            assert "Error: No certificate file specified" in response, response

    @allure.title("CLI. Fetch requires a private-key path for custom mTLS.")
    def test_custom_auth_requires_private_key(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command(
            "fetch -a cert -C /ext/missing.pem https://example.invalid"
        )
        with allure.step("Verify missing private-key-path diagnostics"):
            assert "Error: No private key file specified" in response, response

    @allure.title("CLI. Fetch rejects a missing custom certificate file.")
    def test_custom_auth_rejects_missing_certificate_file(
        self, persistent_cli_connection
    ):
        response = persistent_cli_connection.execute_command(
            "fetch -a cert -C /ext/missing.pem -K /ext/missing.key "
            "https://example.invalid"
        )
        with allure.step("Verify missing certificate-file diagnostics"):
            assert "Error: Certificate file does not exist" in response, response

    @allure.title("CLI. Fetch rejects a missing custom private-key file.")
    def test_custom_auth_rejects_missing_private_key_file(
        self,
        persistent_cli_connection,
        mtls_material,
        storage_api,
        storage_dir,
    ):
        certificate_path = f"{storage_dir}/client.pem"
        upload = storage_api.write(
            certificate_path, mtls_material.valid_client_certificate_pem
        )
        with allure.step("Upload the client certificate without a private key"):
            assert (
                upload.status_code == 200
            ), f"certificate upload status={upload.status_code}, body={upload.text!r}"

        response = persistent_cli_connection.execute_command(
            f"fetch -a cert -C {certificate_path} -K /ext/missing.key "
            "https://example.invalid"
        )
        with allure.step("Verify missing private-key-file diagnostics"):
            assert "Error: Private key file does not exist" in response, response


@allure.epic("BSB CLI Testing")
@allure.feature("Fetch mTLS")
@pytest.mark.uses_si917
class TestFetchCustomMTLS:
    @allure.title("CLI. Fetch authenticates with a custom client certificate.")
    @pytest.mark.parametrize("tls_version", ["TLSv1.2", "TLSv1.3"])
    def test_custom_certificate_success(
        self,
        tls_version,
        persistent_cli_connection,
        device_wifi_ready,
        mtls_material,
        mtls_server_factory,
        upload_mtls_credentials,
    ):
        server = mtls_server_factory(
            mtls_material.trusted_client_ca_pem, tls_version=tls_version
        )
        certificate_path, key_path = upload_mtls_credentials(
            mtls_material.valid_client_certificate_pem,
            mtls_material.valid_client_private_key_pem,
        )

        response = persistent_cli_connection.execute_command(
            f"fetch -v -k -a cert -C {certificate_path} -K {key_path} "
            f"{server.url('/request')}",
            timeout=25,
            slow_command=True,
        )

        with allure.step(f"Verify successful custom mTLS over {tls_version}"):
            assert "HTTP/1.0 200 OK" in response, response

        captured = _get_captured_request(server, response)
        with allure.step("Verify negotiated TLS version and client identity"):
            assert (
                captured["tls_version"] == tls_version
            ), f"expected TLS version={tls_version!r}, captured={captured!r}"
            peer = x509.load_der_x509_certificate(captured["peer_certificate"])
            cn = peer.subject.get_attributes_for_oid(NameOID.COMMON_NAME)
            assert (
                cn and cn[0].value == "Fetch mTLS test client"
            ), f"unexpected peer subject={peer.subject!r}, captured={captured!r}"

    @allure.title("CLI. Fetch presents a custom intermediate certificate chain.")
    def test_custom_certificate_chain_success(
        self,
        persistent_cli_connection,
        device_wifi_ready,
        mtls_material,
        mtls_server_factory,
        upload_mtls_credentials,
    ):
        server = mtls_server_factory(mtls_material.trusted_client_ca_pem)
        certificate_path, key_path = upload_mtls_credentials(
            mtls_material.chained_client_certificate_pem,
            mtls_material.chained_client_private_key_pem,
        )

        response = persistent_cli_connection.execute_command(
            f"fetch -v -k -a cert -C {certificate_path} -K {key_path} "
            f"{server.url('/request')}",
            timeout=25,
            slow_command=True,
        )

        with allure.step("Verify custom intermediate-chain mTLS succeeds"):
            assert "HTTP/1.0 200 OK" in response, response

        captured = _get_captured_request(server, response)
        with allure.step("Verify the server captured the client leaf certificate"):
            assert captured[
                "peer_certificate"
            ], f"server captured no client leaf certificate: {captured!r}"

    @allure.title("CLI. Fetch is rejected when no client certificate is provided.")
    def test_server_rejects_missing_client_certificate(
        self,
        persistent_cli_connection,
        device_wifi_ready,
        mtls_material,
        mtls_server_factory,
    ):
        server = mtls_server_factory(mtls_material.trusted_client_ca_pem)
        response = persistent_cli_connection.execute_command(
            f"fetch -v -k -a none {server.url('/request')}",
            timeout=25,
            slow_command=True,
        )
        _assert_tls_rejected(persistent_cli_connection, response, server)

    @allure.title("CLI. Fetch is rejected for an untrusted custom certificate.")
    def test_server_rejects_untrusted_client_certificate(
        self,
        persistent_cli_connection,
        device_wifi_ready,
        mtls_material,
        mtls_server_factory,
        upload_mtls_credentials,
    ):
        server = mtls_server_factory(mtls_material.trusted_client_ca_pem)
        certificate_path, key_path = upload_mtls_credentials(
            mtls_material.untrusted_client_certificate_pem,
            mtls_material.untrusted_client_private_key_pem,
        )
        response = persistent_cli_connection.execute_command(
            f"fetch -v -k -a cert -C {certificate_path} -K {key_path} "
            f"{server.url('/request')}",
            timeout=25,
            slow_command=True,
        )
        _assert_tls_rejected(persistent_cli_connection, response, server)

    @allure.title("CLI. Fetch is rejected for a client certificate with wrong EKU.")
    def test_server_rejects_wrong_eku_client_certificate(
        self,
        persistent_cli_connection,
        device_wifi_ready,
        mtls_material,
        mtls_server_factory,
        upload_mtls_credentials,
    ):
        server = mtls_server_factory(mtls_material.trusted_client_ca_pem)
        certificate_path, key_path = upload_mtls_credentials(
            mtls_material.wrong_eku_client_certificate_pem,
            mtls_material.wrong_eku_client_private_key_pem,
        )
        response = persistent_cli_connection.execute_command(
            f"fetch -v -k -a cert -C {certificate_path} -K {key_path} "
            f"{server.url('/request')}",
            timeout=25,
            slow_command=True,
        )
        _assert_tls_rejected(persistent_cli_connection, response, server)

    @allure.title("CLI. Fetch rejects invalid custom certificate material.")
    @pytest.mark.parametrize(
        "case",
        [
            "empty-certificate",
            "malformed-certificate",
            "empty-private-key",
            "malformed-private-key",
            "encrypted-private-key",
        ],
    )
    def test_custom_certificate_parse_failure(
        self,
        case,
        persistent_cli_connection,
        device_wifi_ready,
        mtls_material,
        mtls_server_factory,
        upload_mtls_credentials,
    ):
        materials = {
            "empty-certificate": (
                b"",
                mtls_material.valid_client_private_key_pem,
            ),
            "malformed-certificate": (
                b"not a certificate\n",
                mtls_material.valid_client_private_key_pem,
            ),
            "empty-private-key": (
                mtls_material.valid_client_certificate_pem,
                b"",
            ),
            "malformed-private-key": (
                mtls_material.valid_client_certificate_pem,
                b"not a private key\n",
            ),
            "encrypted-private-key": (
                mtls_material.valid_client_certificate_pem,
                mtls_material.encrypted_client_private_key_pem,
            ),
        }
        server = mtls_server_factory(mtls_material.trusted_client_ca_pem)
        certificate_path, key_path = upload_mtls_credentials(*materials[case])

        response = persistent_cli_connection.execute_command(
            f"fetch -v -k -a cert -C {certificate_path} -K {key_path} "
            f"{server.url('/request')}",
            timeout=25,
            slow_command=True,
        )

        with allure.step(f"Verify TLS material parse failure for {case}"):
            assert "Error: Failed to establish TLS connection" in response, response
        _assert_tls_rejected(persistent_cli_connection, response, server)

    @allure.title("CLI. Fetch is rejected for an invalid client validity period.")
    @pytest.mark.parametrize("case", ["expired", "not-yet-valid"])
    def test_server_rejects_invalid_client_validity(
        self,
        case,
        persistent_cli_connection,
        device_wifi_ready,
        mtls_material,
        mtls_server_factory,
        upload_mtls_credentials,
    ):
        materials = {
            "expired": (
                mtls_material.expired_client_certificate_pem,
                mtls_material.expired_client_private_key_pem,
            ),
            "not-yet-valid": (
                mtls_material.future_client_certificate_pem,
                mtls_material.future_client_private_key_pem,
            ),
        }
        server = mtls_server_factory(mtls_material.trusted_client_ca_pem)
        certificate_path, key_path = upload_mtls_credentials(*materials[case])

        response = persistent_cli_connection.execute_command(
            f"fetch -v -k -a cert -C {certificate_path} -K {key_path} "
            f"{server.url('/request')}",
            timeout=25,
            slow_command=True,
        )

        _assert_tls_rejected(persistent_cli_connection, response, server)

    @allure.title("CLI. Fetch rejects a certificate/private-key mismatch.")
    def test_custom_certificate_key_mismatch(
        self,
        persistent_cli_connection,
        device_wifi_ready,
        mtls_material,
        mtls_server_factory,
        upload_mtls_credentials,
    ):
        server = mtls_server_factory(mtls_material.trusted_client_ca_pem)
        certificate_path, key_path = upload_mtls_credentials(
            mtls_material.valid_client_certificate_pem,
            mtls_material.mismatched_client_private_key_pem,
        )
        response = persistent_cli_connection.execute_command(
            f"fetch -v -k -a cert -C {certificate_path} -K {key_path} "
            f"{server.url('/request')}",
            timeout=25,
            slow_command=True,
        )
        _assert_tls_rejected(persistent_cli_connection, response, server)

    @allure.title("CLI. Fetch rejects an untrusted server with a matching IP SAN.")
    def test_server_certificate_verification_is_enabled(
        self,
        persistent_cli_connection,
        device_wifi_ready,
        mtls_material,
        mtls_server_factory,
        upload_mtls_credentials,
    ):
        server = mtls_server_factory(mtls_material.trusted_client_ca_pem)
        certificate_path, key_path = upload_mtls_credentials(
            mtls_material.valid_client_certificate_pem,
            mtls_material.valid_client_private_key_pem,
        )
        with allure.step("Verify hostname matching cannot cause this rejection"):
            san = server.server_certificate.extensions.get_extension_for_class(
                x509.SubjectAlternativeName
            ).value
            san_addresses = [
                str(value) for value in san.get_values_for_type(x509.IPAddress)
            ]
            server_host = server.server_address[0]
            assert (
                server_host in san_addresses
            ), f"server host={server_host!r}, certificate IP SANs={san_addresses!r}"

        response = persistent_cli_connection.execute_command(
            f"fetch -v -a cert -C {certificate_path} -K {key_path} "
            f"{server.url('/request')}",
            timeout=25,
            slow_command=True,
        )
        _assert_tls_rejected(persistent_cli_connection, response, server)


@allure.epic("BSB CLI Testing")
@allure.feature("Fetch mTLS")
@pytest.mark.uses_si917
class TestFetchDeviceMTLS:
    @allure.title("CLI. Provisioned device mTLS is accepted by the development API.")
    @pytest.mark.external_service
    @pytest.mark.regression
    def test_provisioned_device_cloud_mtls_success(
        self,
        persistent_cli_connection,
        device_wifi_ready,
        device_mtls_identity,
    ):
        response = persistent_cli_connection.execute_command(
            f'fetch -v -a device "{CLOUD_WEATHER_URL}"',
            timeout=35,
            slow_command=True,
        )

        with allure.step("Verify the trusted TLS request reached the dev backend"):
            assert "wrong_eku" not in response, response
            status_match = re.search(r"HTTP/\d(?:\.\d)? (\d{3})\b", response)
            assert status_match, f"dev backend returned no HTTP status: {response!r}"

        if status_match.group(1) == "403" and response.rstrip().endswith("null"):
            with allure.step("Identify an unprovisioned bench device response"):
                assert "x-request-id:" in response.casefold(), response
            pytest.skip("cloud mTLS E2E requires a registered/authorized bench device")

        with allure.step("Verify the provisioned device is accepted over mTLS"):
            assert status_match.group(1) == "200", (
                f"expected cloud HTTP 200, status={status_match.group(1)}, "
                f"response={response!r}"
            )

    @allure.title("CLI. Fetch authenticates with the hardware-backed device key.")
    def test_device_certificate_success(
        self,
        persistent_cli_connection,
        device_wifi_ready,
        mtls_server_factory,
        device_mtls_identity,
        system_api,
    ):
        device_info = system_api.get_device_info()
        # F21 OTP metadata and the Si917 mTLS identity are provisioned separately.
        # Record the former for diagnostics, but gate this test on the crypto slots
        # and certificate identity validated by ``device_mtls_identity``.
        allure.dynamic.parameter("otp_valid", device_info.otp_valid)
        with allure.step("Verify the device certificate belongs to this hardware"):
            assert device_mtls_identity.hardware_id.casefold() == (
                device_info.serial_number.casefold()
            ), (
                f"certificate hardware ID={device_mtls_identity.hardware_id!r}, "
                f"device serial={device_info.serial_number!r}"
            )

        intermediate_pem = device_mtls_identity.intermediate.public_bytes(
            encoding=serialization.Encoding.PEM
        )
        server = mtls_server_factory(intermediate_pem, allow_partial_chain=True)

        response = persistent_cli_connection.execute_command(
            f"fetch -v -k -a device {server.url('/request')}",
            timeout=25,
            slow_command=True,
        )

        with allure.step("Verify hardware-backed device mTLS succeeds"):
            assert "HTTP/1.0 200 OK" in response, response

        captured = _get_captured_request(server, response)
        with allure.step("Verify the exact device leaf certificate was presented"):
            peer = x509.load_der_x509_certificate(captured["peer_certificate"])
            actual_der = peer.public_bytes(serialization.Encoding.DER)
            expected_der = device_mtls_identity.certificate.public_bytes(
                serialization.Encoding.DER
            )
            assert actual_der == expected_der, (
                f"presented device certificate differs: expected={expected_der.hex()}, "
                f"actual={actual_der.hex()}"
            )

    @allure.title("CLI. Shared CA bundle survives device mTLS context teardown.")
    @pytest.mark.external_service
    @pytest.mark.regression
    def test_ca_bundle_survives_device_mtls_teardown(
        self,
        persistent_cli_connection,
        device_wifi_ready,
        mtls_server_factory,
        device_mtls_identity,
        storage_dir,
    ):
        intermediate_pem = device_mtls_identity.intermediate.public_bytes(
            encoding=serialization.Encoding.PEM
        )
        server = mtls_server_factory(intermediate_pem, allow_partial_chain=True)
        local_response = persistent_cli_connection.execute_command(
            f"fetch -v -k -a device {server.url('/request')}",
            timeout=25,
            slow_command=True,
        )

        with allure.step("Create and tear down a hardware-backed mTLS context"):
            assert "HTTP/1.0 200 OK" in local_response, local_response

        probe_path = f"{storage_dir}/ca-bundle-probe.json"
        trusted_response = persistent_cli_connection.execute_command(
            f'fetch -v -a none -o {probe_path} "{PUBLIC_CA_PROBE_URL}"',
            timeout=35,
            slow_command=True,
        )

        with allure.step("Verify a subsequent trusted TLS handshake and download"):
            assert re.search(
                r"HTTP/\d(?:\.\d)? 200\b", trusted_response
            ), f"trusted CA probe failed after mTLS teardown: {trusted_response!r}"
            stat = persistent_cli_connection.execute_command(
                f"storage stat {probe_path}"
            )
            assert re.search(r"size: [1-9]\d*b", stat), (
                f"trusted CA probe produced no payload: stat={stat!r}, "
                f"Fetch output={trusted_response!r}"
            )

    @allure.title("CLI. Fetch mTLS does not interrupt an active MQTT mTLS session.")
    @pytest.mark.mqtt
    def test_device_fetch_preserves_mqtt_mtls_session(
        self,
        persistent_cli_connection,
        device_wifi_ready,
        mtls_server_factory,
        mtls_mqtt_broker_factory,
        fetch_mtls_account_backend_guard,
        device_mtls_identity,
        account_api,
    ):
        intermediate_pem = device_mtls_identity.intermediate.public_bytes(
            encoding=serialization.Encoding.PEM
        )
        broker = mtls_mqtt_broker_factory(intermediate_pem, allow_partial_chain=True)
        account_api.set_backend(
            AccountBackend(
                server_url=broker.url,
                client_cert_type="default",
                ignore_server_cert=True,
            )
        )

        mqtt_state, _ = wait_for(
            "device MQTT session to become connected",
            lambda: (account_api.get_status(), broker.diagnostics()),
            lambda state: state[0].status == "connected",
            timeout=MQTT_CONNECTION_TIMEOUT_SECONDS,
            interval=0.5,
        )
        with allure.step("Verify MQTT v5 connected with the device certificate"):
            diagnostics = broker.diagnostics()
            assert (
                mqtt_state.status == "connected"
            ), f"MQTT status={mqtt_state.status!r}, broker={diagnostics!r}"
            assert broker.connected.wait(
                timeout=1
            ), f"broker received no MQTT CONNECT: {diagnostics!r}"
            protocol_level = broker.connect_protocol_levels.get(timeout=1)
            assert (
                protocol_level == 5
            ), f"expected MQTT protocol level 5, got {protocol_level!r}"
            mqtt_peer = x509.load_der_x509_certificate(
                broker.peer_certificates.get(timeout=1)
            )
            actual_der = mqtt_peer.public_bytes(serialization.Encoding.DER)
            expected_der = device_mtls_identity.certificate.public_bytes(
                serialization.Encoding.DER
            )
            assert actual_der == expected_der, (
                f"MQTT peer certificate differs: expected={expected_der.hex()}, "
                f"actual={actual_der.hex()}"
            )

        server = mtls_server_factory(intermediate_pem, allow_partial_chain=True)
        response = persistent_cli_connection.execute_command(
            f"fetch -v -k -a device {server.url('/request')}",
            timeout=25,
            slow_command=True,
        )

        with allure.step("Verify Fetch succeeded without disturbing MQTT"):
            assert "HTTP/1.0 200 OK" in response, response
            mqtt_status = account_api.get_status().status
            diagnostics = broker.diagnostics()
            assert (
                mqtt_status == "connected"
            ), f"MQTT status after Fetch={mqtt_status!r}, broker={diagnostics!r}"
            assert (
                diagnostics["connections"] == 1
            ), f"MQTT reconnected while Fetch was running: {diagnostics!r}"
            assert (
                diagnostics["disconnects"] == 0
            ), f"MQTT disconnected while Fetch was running: {diagnostics!r}"
            assert not diagnostics[
                "errors"
            ], f"local MQTT broker errors={diagnostics['errors']!r}"
