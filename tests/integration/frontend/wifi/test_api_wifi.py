import json
import time
from time import sleep

import allure
import pytest
import requests

from clients.api import APIError, TEST_WIFI_SSID, WifiAPI


def wait_for_wifi_state(wifi_api: WifiAPI, states: list[str], timeout: int = 20) -> str:
    """Poll WiFi status until it matches one of the expected states."""
    deadline = time.time() + timeout
    last_state = ""
    while time.time() < deadline:
        last_state = wifi_api.get_status().state
        if last_state in states:
            return last_state
        sleep(1)
    pytest.fail(f"Timed out waiting for WiFi state {states}, last state: {last_state}")


def ensure_disconnected(wifi_api: WifiAPI, timeout: int = 20) -> None:
    """Bring the device into the `disconnected` state, tolerating transitions.
    """
    if wifi_api.get_status().state == "disconnected":
        return
    try:
        wifi_api.disconnect()
    except APIError:
        pass
    wait_for_wifi_state(wifi_api, ["disconnected"], timeout=timeout)


def connect_to_test_network_or_fail(wifi_api: WifiAPI, timeout: int = 30) -> None:
    """Connect to the test SSID and fail with diagnostics if the API rejects it.
    """
    response = wifi_api.connect_to_test_network(timeout=timeout)
    if response.status_code != 200:
        body = response.text.strip() or "(empty body)"
        pytest.fail(
            f"POST /api/wifi/connect to {TEST_WIFI_SSID!r} failed: "
            f"HTTP {response.status_code} — {body}"
        )
    wait_for_wifi_state(wifi_api, ["connected"], timeout=timeout)


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
        if TEST_WIFI_SSID not in ssids:
            pytest.skip(f"Test SSID not found in scan results: {TEST_WIFI_SSID}")

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

    @allure.title("WiFi regression: connect → disconnect → scan x3")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.regression
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
