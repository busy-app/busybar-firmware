import hashlib
import json
import re
import time
import uuid
from time import sleep
from urllib.parse import urlsplit

import allure
import pytest
import requests

from clients.api import WIFI_SSID, SettingsAPI, WifiAPI
from utils.wifi_helpers import (
    connect_to_test_network_or_fail,
    ensure_disconnected,
    fetch_over_wifi_with_recovery,
    wait_for_wifi_link_stable,
    wait_for_wifi_state,
)


_WIFI_API_ACCESS_KEY = "12345678"


def wifi_external_base_url_or_fail(wifi_api: WifiAPI, web_base_url: str) -> str:
    status = wait_for_wifi_link_stable(wifi_api)
    ip_config = status.ip_config
    if status.state != "connected" or not ip_config or not ip_config.address:
        pytest.fail(
            "WiFi is connected but no IP address was reported: "
            f"state={status.state!r}, ip_config={ip_config!r}"
        )

    address = ip_config.address.split("/", 1)[0]
    if not address or address == "0.0.0.0":
        pytest.fail(f"WiFi reported unusable IP address: {ip_config.address!r}")

    local_host = urlsplit(web_base_url).hostname
    if address == local_host or address.startswith("127."):
        pytest.fail(
            "WiFi status did not report an external WiFi address: "
            f"address={address!r}, WEB_BASE_URL={web_base_url!r}"
        )

    allure.attach(
        json.dumps(
            {
                "state": status.state,
                "ssid": status.ssid,
                "address": ip_config.address,
                "local_base_url": web_base_url,
                "wifi_external_base_url": f"http://{address}",
            },
            indent=2,
        ),
        name="Connected WiFi external API address",
        attachment_type=allure.attachment_type.JSON,
    )
    return f"http://{address}"


def wifi_get(
    web_session: requests.Session,
    base_url: str,
    endpoint: str,
    token: str | None = None,
) -> requests.Response:
    url = f"{base_url}{endpoint}"
    headers = {
        "User-Agent": "BSB-AutoTest/1.0",
        "Accept": "application/json",
    }
    if token is not None:
        headers["X-API-Token"] = token

    response = web_session.get(url, headers=headers, timeout=10)
    body = response.text

    allure.attach(
        json.dumps(
            {
                "endpoint": endpoint,
                "status_code": response.status_code,
                "token_present": token is not None,
                "body": body,
            },
            indent=2,
        ),
        name=f"WiFi GET {endpoint}",
        attachment_type=allure.attachment_type.JSON,
    )
    return response


@pytest.fixture(scope="module", autouse=True)
def wifi_setup_teardown(web_base_url):
    """Fixture to reconnect to known WiFi after tests"""
    yield
    # Teardown: Reconnect to known WiFi using a fresh session
    session = requests.Session()
    session.headers.update(
        {"User-Agent": "BSB-AutoTest/1.0", "Accept": "application/json"}
    )
    try:
        wifi_api = WifiAPI(session, web_base_url)
        wifi_api.connect_to_test_network()
    finally:
        session.close()


@allure.feature("5. Web Frontend")
@allure.story("Wi-Fi")
@pytest.mark.regression
class TestWifiAPI:
    """Test cases for WiFi API endpoints"""

    @allure.id("2657")
    @allure.title("GET /api/wifi/status")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_status(self, wifi_api: WifiAPI):
        """Test GET /api/wifi/status endpoint"""
        with allure.step("Check initial WiFi status"):
            initial_status = wifi_api.get_status()
            allure.attach(
                json.dumps({"state": initial_status.state}, indent=2),
                name="Initial WiFi Status",
                attachment_type=allure.attachment_type.JSON
            )

        with allure.step("Disconnect first (force a known starting state)"):
            ensure_disconnected(wifi_api)

        with allure.step("Connect to test network"):
            connect_to_test_network_or_fail(wifi_api)
            assert wifi_api.get_status().state == "connected"

    @allure.id("2660")
    @allure.title("GET /api/wifi/networks")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_networks(self, wifi_api: WifiAPI):
        """Test GET /api/wifi/networks endpoint"""
        if wifi_api.get_status().state in ["connected", "connecting"]:
            wifi_api.disconnect()
        response = wifi_api.get_networks(timeout=10)

        assert isinstance(response.networks, list)
        assert response.count >= 0

        # Check that test network is in the list
        ssids = [network.ssid for network in response.networks]
        if WIFI_SSID not in ssids:
            pytest.skip(f"Test SSID not found in scan results: {WIFI_SSID}")

    @allure.id("2661")
    @allure.title("POST /api/wifi/connect")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_connect_invalid(self, wifi_api: WifiAPI):
        """Test POST /api/wifi/connect endpoint with invalid data"""
        response = wifi_api.connect(
            ssid="NonExistentNetwork",
            password="wrongpassword",
            security="WPA2",
            timeout=30,
        )

        # Should return 400 for bad request
        assert response.status_code in [400]

    @allure.id("2662")
    @allure.title("POST /api/wifi/disconnect")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_disconnect(self, wifi_api: WifiAPI):
        """Test POST /api/wifi/disconnect endpoint"""
        with allure.step("Ensure connected to test network before disconnect"):
            status = wifi_api.get_status()
            allure.attach(
                json.dumps({"state": status.state}, indent=2),
                name="WiFi Status Before Disconnect",
                attachment_type=allure.attachment_type.JSON,
            )
            if status.state != "connected":
                ensure_disconnected(wifi_api)
                connect_to_test_network_or_fail(wifi_api)

        wifi_api.disconnect()
        wait_for_wifi_state(wifi_api, ["disconnected"], timeout=20)

    @allure.title("WiFi regression: access key auth works over Wi-Fi API")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_access_key_after_connect(
        self,
        wifi_api: WifiAPI,
        settings_api: SettingsAPI,
        web_session: requests.Session,
        web_base_url: str,
    ):
        original_access = settings_api.get_access()

        with allure.step("Use default test network connection"):
            status = wifi_api.get_status()
            connected_to_test_network = (
                status.state == "connected"
                and (not WIFI_SSID or status.ssid in (None, WIFI_SSID))
            )
            if not connected_to_test_network:
                ensure_disconnected(wifi_api)
                connect_to_test_network_or_fail(wifi_api)
            wifi_base_url = wifi_external_base_url_or_fail(wifi_api, web_base_url)

        try:
            with allure.step("Require access key for HTTP API over Wi-Fi"):
                settings_api.set_access("key", _WIFI_API_ACCESS_KEY)
                verify = settings_api.get_access()
                assert verify.mode == "key"
                assert verify.key_valid is True

            with allure.step("Verify request path is Wi-Fi"):
                transport = wifi_get(web_session, wifi_base_url, "/api/transport")
                assert transport.status_code == 200
                assert transport.json()["type"] == "wifi"

            with allure.step("Reject protected API without or with wrong key"):
                assert (
                    wifi_get(web_session, wifi_base_url, "/api/status").status_code
                    == 403
                )
                assert (
                    wifi_get(
                        web_session,
                        wifi_base_url,
                        "/api/status",
                        token="00000000",
                    ).status_code
                    == 403
                )

            with allure.step("Allow protected API with valid key"):
                assert (
                    wifi_get(
                        web_session,
                        wifi_base_url,
                        "/api/status",
                        token=_WIFI_API_ACCESS_KEY,
                    ).status_code
                    == 200
                )
                status = wifi_get(
                    web_session,
                    wifi_base_url,
                    "/api/wifi/status",
                    token=_WIFI_API_ACCESS_KEY,
                )
                assert status.status_code == 200
                assert status.json()["state"] == "connected"
        finally:
            if original_access.mode == "key":
                settings_api.set_access("key", _WIFI_API_ACCESS_KEY)
            else:
                settings_api.set_access(original_access.mode)

    @allure.title("WiFi regression: connect → disconnect → scan x3")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_connect_disconnect_then_repeated_scan(self, wifi_api: WifiAPI):
        """Reproduces the UI flow: connect, disconnect, then repeatedly press
        'Select network' (= GET /api/wifi/networks).

        The firmware should remain responsive across multiple back-to-back
        scans following a disconnect — historically this path has wedged the
        device for a couple of minutes.
        """
        with allure.step("Connect to test network"):
            ensure_disconnected(wifi_api)
            connect_to_test_network_or_fail(wifi_api)

        with allure.step("Disconnect"):
            wifi_api.disconnect()
            wait_for_wifi_state(wifi_api, ["disconnected"], timeout=30)

        scan_attempts = 3
        scan_pause = 5
        for attempt in range(1, scan_attempts + 1):
            with allure.step(f"Scan attempt {attempt}/{scan_attempts}"):
                t0 = time.time()
                response = wifi_api.get_networks(timeout=60)
                elapsed = time.time() - t0

                allure.attach(
                    json.dumps(
                        {
                            "attempt": attempt,
                            "elapsed_s": round(elapsed, 2),
                            "count": response.count,
                            "ssids": [n.ssid for n in response.networks],
                        },
                        indent=2,
                    ),
                    name=f"Scan #{attempt}",
                    attachment_type=allure.attachment_type.JSON,
                )

                assert isinstance(response.networks, list), (
                    f"Scan #{attempt}: expected list, got {type(response.networks)}"
                )
                assert response.count >= 0, (
                    f"Scan #{attempt}: negative count {response.count}"
                )

                state_after = wifi_api.get_status().state
                assert state_after == "disconnected", (
                    f"Scan #{attempt} flipped state to {state_after!r}"
                )

            if attempt < scan_attempts:
                sleep(scan_pause)


@allure.feature("5. Web Frontend")
@allure.story("Wi-Fi / fetch")
class TestWifiFetch:
    """`fetch` CLI downloads a URL into device storage over the Wi-Fi link."""

    _UPDATE_DIR = "https://update.busy.app/busybar-firmware/directory.json"

    @allure.title("GET /api/wifi/status reports connected state with IP")
    @pytest.mark.frontend
    def test_wifi_status_after_connect(self, wifi_api):
        if wifi_api.get_status().state != "connected":
            status = connect_to_test_network_or_fail(wifi_api)
        else:
            status = wait_for_wifi_link_stable(wifi_api)

        with allure.step("Verify the stable Wi-Fi connection and IP address"):
            assert status.state == "connected", f"Wi-Fi status={status!r}"
            assert (
                status.ip_config and status.ip_config.address
            ), f"Wi-Fi has no usable IP configuration: status={status!r}"

    @allure.title("CLI. fetch completes over HTTPS with bounded Wi-Fi recovery")
    @pytest.mark.cli
    @pytest.mark.frontend
    @pytest.mark.external_service
    def test_cli_fetch_smoke(self, persistent_cli_connection, wifi_api):
        if wifi_api.get_status().state != "connected":
            connect_to_test_network_or_fail(wifi_api)

        result = fetch_over_wifi_with_recovery(
            persistent_cli_connection,
            wifi_api,
            self._UPDATE_DIR,
        )
        with allure.step("Verify Fetch completed after Wi-Fi became usable"):
            assert (
                "Downloaded: 100%" in result.output
            ), f"Fetch attempts={result.attempt_count}, output={result.output!r}"

    @allure.title("CLI. fetch + storage md5 matches runner-side md5")
    @pytest.mark.cli
    @pytest.mark.frontend
    @pytest.mark.regression
    @pytest.mark.external_service
    def test_cli_fetch_md5_matches(
        self,
        persistent_cli_connection,
        wifi_api,
        web_session: requests.Session,
    ):
        if wifi_api.get_status().state != "connected":
            connect_to_test_network_or_fail(wifi_api)
        else:
            wait_for_wifi_link_stable(wifi_api)

        directory = web_session.get(self._UPDATE_DIR, timeout=10).json()
        url = next(
            f["url"]
            for ch in directory["channels"]
            for v in ch["versions"]
            for f in v["files"]
            if f["url"].endswith(".bin")
        )
        expected_md5 = hashlib.md5(
            web_session.get(url, timeout=120).content
        ).hexdigest()

        path = f"/ext/_test_fetch_{uuid.uuid4().hex[:8]}.bin"
        try:
            out = persistent_cli_connection.execute_command(
                f"fetch {url} -o {path}", timeout=300, slow_command=True,
            )
            assert "Downloaded: 100%" in out, out

            md5_out = persistent_cli_connection.execute_command(
                f"storage md5 {path}", timeout=60, slow_command=True,
            )
            match = re.search(r"\b([0-9a-f]{32})\b", md5_out)
            assert match, f"no md5 in `storage md5` output:\n{md5_out}"
            assert match.group(1) == expected_md5, (
                f"md5 mismatch: device={match.group(1)} runner={expected_md5}"
            )
        finally:
            persistent_cli_connection.execute_command(f"storage remove {path}")
