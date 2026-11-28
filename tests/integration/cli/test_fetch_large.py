"""Large HTTP downloads through the device Fetch implementation."""

import hashlib
import queue

import allure
import pytest

from clients.api import StorageAPI
from utils.fetch_http_server import LARGE_CLOSE_PAYLOAD


pytestmark = [
    pytest.mark.cli,
    pytest.mark.bsb_metadata(
        suite="Integration",
        level="Integration",
        intent="Reliability",
        interfaces=("CLI", "Device API"),
        transports=("Telnet", "LAN REST"),
    ),
]


@allure.epic("Device Connectivity")
@allure.feature("Outbound HTTP/TLS")
@allure.title("CLI. Command fetch saves all 512 KiB of a close-delimited response.")
@allure.issue("https://flipper.atlassian.net/browse/FW-1348")
@pytest.mark.xfail(
    reason="FW-1348: lwIP can lose EOF when the receive mailbox is full",
    raises=AssertionError,
    strict=False,
)
def test_large_http_download_without_content_length(
    persistent_cli_connection,
    http_server,
    storage_api: StorageAPI,
    storage_dir,
):
    destination = f"{storage_dir}/large-http-eof.bin"
    request_path = "/http11/large-close-512k"
    with allure.step("Download a large HTTP response delimited by connection close"):
        output = persistent_cli_connection.execute_command(
            f"fetch -v -o {destination} {http_server.url(request_path)}",
            timeout=30,
            slow_command=True,
        )
        allure.attach(output, "Fetch output", allure.attachment_type.TEXT)

    with allure.step("Verify HTTP request and close-delimited response framing"):
        try:
            captured = http_server.requests.get(timeout=1)
        except queue.Empty:
            pytest.fail(f"No request for {request_path!r}; Fetch output={output!r}")
        assert captured["method"] == "GET", captured
        assert captured["path"] == request_path, captured
        assert captured["version"] == "HTTP/1.1", captured
        assert "HTTP/1.1 200 OK" in output, output
        assert "Content-Length" not in output, output
        assert "Transfer-Encoding" not in output, output
        assert "Error:" not in output, output

    with allure.step("Verify downloaded file size and checksum"):
        saved = storage_api.read(destination)
        assert (
            saved.status_code == 200
        ), f"Downloaded file is missing: HTTP {saved.status_code}"
        assert len(saved.content) == len(LARGE_CLOSE_PAYLOAD), len(saved.content)
        assert (
            hashlib.sha256(saved.content).digest()
            == hashlib.sha256(LARGE_CLOSE_PAYLOAD).digest()
        ), "Downloaded HTTP payload checksum differs"
