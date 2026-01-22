import json
from time import sleep

import allure
import pytest
import requests

from clients.api import TEST_WIFI_SSID, WifiAPI


@pytest.fixture(scope="module", autouse=True)
def wifi_setup_teardown(web_base_url):
    """Fixture to reconnect to known WiFi after tests"""
    yield
    # Teardown: Reconnect to known WiFi using a fresh session
    session = requests.Session()
    session.headers.update({"Accept": "application/json"})
    wifi_api = WifiAPI(session, web_base_url)
    wifi_api.connect_to_test_network()


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

        with allure.step("Disconnect if connected"):
            if initial_status.state in ["connected", "connecting"]:
                wifi_api.disconnect()
                response = wifi_api.get_status()
                assert response.state in ["disconnecting", "disconnected"]

                sleep(2)  # Wait for state to update
                response = wifi_api.get_status()
                assert response.state == "disconnected"
            else:
                # Already disconnected
                assert initial_status.state in ["disconnected", "unknown"]

        with allure.step("Connect to test network"):
            wifi_api.connect_to_test_network()
            response = wifi_api.get_status()
            assert response.state in ["connected", "connecting"]

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
        assert TEST_WIFI_SSID in ssids

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
        with allure.step("Check WiFi status before disconnect"):
            status = wifi_api.get_status()
            allure.attach(json.dumps({"state": status.state}, indent=2), name="WiFi Status Before Disconnect", attachment_type=allure.attachment_type.JSON)

            if status.state not in ["connected", "connecting"]:
                wifi_api.connect_to_test_network()

        wifi_api.disconnect()
        sleep(2)  # Wait for state to update
        status = wifi_api.get_status()
        assert status.state == "disconnected"
