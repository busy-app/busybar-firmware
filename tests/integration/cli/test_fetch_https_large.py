"""Large TLS responses must complete through the device's Wi-Fi interface."""

import hashlib
import queue

import allure
import pytest

from clients.api import StorageAPI
from utils.fetch_large_https_server import LARGE_HTTPS_PAYLOAD


pytestmark = [
    pytest.mark.cli,
    pytest.mark.uses_si917,
    pytest.mark.bsb_metadata(
        suite="Integration",
        level="Integration",
        intent="Reliability",
        interfaces=("CLI", "Device API"),
        transports=("Telnet", "LAN REST"),
        hardware=("BusyBar", "Wi-Fi AP"),
    ),
]


@allure.epic("Device Connectivity")
@allure.feature("Outbound HTTP/TLS")
@allure.title("Wi-Fi HTTPS saves all 64 KiB: {tls_version}, {framing}")
@pytest.mark.parametrize(
    "tls_version,framing",
    [
        pytest.param("TLSv1.2", "length", id="tls12-length"),
        pytest.param("TLSv1.3", "length", id="tls13-length"),
        pytest.param("TLSv1.3", "chunked", id="tls13-chunked"),
        pytest.param("TLSv1.3", "close", id="tls13-eof"),
    ],
)
def test_large_https_download_over_wifi(
    persistent_cli_connection,
    wifi_https_server,
    storage_api: StorageAPI,
    storage_dir,
    tls_version,
    framing,
):
    server, wifi_ip = wifi_https_server
    destination = f"{storage_dir}/large-https.bin"
    with allure.step("Download a bounded binary response over Wi-Fi"):
        # Trust validation has separate tests; this checks TLS body delivery/framing.
        output = persistent_cli_connection.execute_command(
            f"fetch -v -k -a none -o {destination} {server.url('/large/' + framing)}",
            timeout=30,
            slow_command=True,
        )
        allure.attach(output, "Fetch output", allure.attachment_type.TEXT)

    with allure.step(
        "Verify the server observed the Wi-Fi peer and selected TLS version"
    ):
        try:
            captured = server.requests.get(timeout=1)
        except queue.Empty:
            pytest.fail(
                "HTTPS server received no request. Check FETCH_WIFI_SERVER_HOST/PORT "
                "and publish the server port on Docker runners; see Fetch output."
            )
        assert captured["peer_ip"] == wifi_ip, captured
        assert captured["tls_version"] == tls_version, captured
        assert captured["framing"] == framing, captured

    with allure.step("Verify successful completion and exact file contents"):
        assert (
            "HTTP/1.1 200 OK" in output
        ), "No successful HTTPS response; see Fetch output"
        assert "Error:" not in output, "HTTPS download failed; see Fetch output"
        saved = storage_api.read(destination)
        assert (
            saved.status_code == 200
        ), f"Downloaded file is missing: HTTP {saved.status_code}"
        assert len(saved.content) == len(LARGE_HTTPS_PAYLOAD), len(saved.content)
        assert (
            hashlib.sha256(saved.content).digest()
            == hashlib.sha256(LARGE_HTTPS_PAYLOAD).digest()
        ), "Downloaded HTTPS payload checksum differs"
