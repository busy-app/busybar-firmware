import json

import allure
import pytest


@allure.feature("5. Web Frontend")
@allure.story("Local API - Wi-Fi")
class TestWifiAPI:
    """Test cases for WiFi API endpoints"""

    @allure.id("2657")
    @allure.title("GET /api/wifi/status")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_status(self, api_session, web_base_url):
        """Test GET /api/wifi/status endpoint"""

        with allure.step("Get WiFi status"):
            response = api_session.get(f"{web_base_url}/api/wifi/status", timeout=10)

        with allure.step("Verify status response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            status_data = response.json()
            allure.attach(
                json.dumps(status_data, indent=2),
                "WiFi Status Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields
            assert "state" in status_data, "Response should contain 'state' field"
            valid_states = [
                "unknown",
                "disconnected",
                "connected",
                "connecting",
                "disconnecting",
                "reconnecting",
            ]
            assert (
                status_data["state"] in valid_states
            ), f"State should be one of {valid_states}"

    @allure.id("2660")
    @allure.title("GET /api/wifi/networks")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_networks(self, api_session, web_base_url):
        """Test GET /api/wifi/networks endpoint"""

        with allure.step("Scan for WiFi networks"):
            response = api_session.get(
                f"{web_base_url}/api/wifi/networks", timeout=30
            )  # Longer timeout for scan

        with allure.step("Verify networks response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            networks_data = response.json()
            allure.attach(
                json.dumps(networks_data, indent=2),
                "WiFi Networks Response",
                allure.attachment_type.JSON,
            )

            assert "count" in networks_data, "Response should contain 'count' field"
            assert (
                "networks" in networks_data
            ), "Response should contain 'networks' field"
            assert isinstance(
                networks_data["networks"], list
            ), "Networks should be a list"

    @allure.id("2661")
    @allure.title("POST /api/wifi/connect")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_connect_invalid(self, api_session, web_base_url):
        """Test POST /api/wifi/connect endpoint with invalid data"""

        with allure.step("Attempt WiFi connection with invalid credentials"):
            connect_data = {
                "ssid": "NonExistentNetwork",
                "password": "wrongpassword",
                "security": "WPA2",
            }
            response = api_session.post(
                f"{web_base_url}/api/wifi/connect", json=connect_data, timeout=30
            )

        with allure.step("Verify connect response"):
            # Should return 400 for bad request or connection failure
            assert response.status_code in [
                200,
                400,
            ], f"Expected 200 or 400, got {response.status_code}"

    @allure.id("2662")
    @allure.title("POST /api/wifi/disconnect")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_disconnect(self, api_session, web_base_url):
        """Test POST /api/wifi/disconnect endpoint"""

        with allure.step("Check WiFi status before disconnect"):
            status_response = api_session.get(
                f"{web_base_url}/api/wifi/status", timeout=10
            )
            assert (
                status_response.status_code == 200
            ), f"Expected 200, got {status_response.status_code}"
            status_data = status_response.json()
            wifi_state = status_data.get("state", "unknown")
            allure.attach(
                json.dumps(status_data, indent=2),
                "WiFi Status Before Disconnect",
                allure.attachment_type.JSON,
            )

            if wifi_state not in ["connected", "connecting"]:
                pytest.skip(f"WiFi is not connected (state: {wifi_state}), skipping disconnect test")

        with allure.step("Disconnect from WiFi"):
            response = api_session.post(
                f"{web_base_url}/api/wifi/disconnect", timeout=10
            )

        with allure.step("Verify disconnect response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "WiFi Disconnect Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"
