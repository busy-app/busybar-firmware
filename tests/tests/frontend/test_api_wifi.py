import allure
import pytest

from utils import api_get, api_post, attach_json, assert_field_in, assert_field_type


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
            response = api_get(api_session, web_base_url, "/api/wifi/status")

        with allure.step("Verify status response"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("state").attach_to_allure("WiFi Status Response")

            # Validate state enum
            valid_states = [
                "unknown", "disconnected", "connected",
                "connecting", "disconnecting", "reconnecting",
            ]
            assert_field_in(response.json(), "state", valid_states)

    @allure.id("2660")
    @allure.title("GET /api/wifi/networks")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_networks(self, api_session, web_base_url):
        """Test GET /api/wifi/networks endpoint"""

        with allure.step("Scan for WiFi networks"):
            response = api_get(api_session, web_base_url, "/api/wifi/networks", timeout=30)

        with allure.step("Verify networks response"):
            response.assert_ok()
            response.assert_has_fields("count", "networks").attach_to_allure("WiFi Networks Response")

            data = response.json()
            assert_field_type(data, "networks", list)

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
            response = api_post(
                api_session, web_base_url, "/api/wifi/connect",
                json=connect_data, timeout=30
            )

        with allure.step("Verify connect response"):
            # Should return 400 for bad request or 200 for connection attempt
            response.assert_status([200, 400])

    @allure.id("2662")
    @allure.title("POST /api/wifi/disconnect")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_disconnect(self, api_session, web_base_url):
        """Test POST /api/wifi/disconnect endpoint"""

        with allure.step("Check WiFi status before disconnect"):
            status_response = api_get(api_session, web_base_url, "/api/wifi/status")
            status_response.assert_ok()

            status_data = status_response.json()
            wifi_state = status_data.get("state", "unknown")
            attach_json(status_data, "WiFi Status Before Disconnect")

            if wifi_state not in ["connected", "connecting"]:
                pytest.skip(f"WiFi is not connected (state: {wifi_state}), skipping disconnect test")

        with allure.step("Disconnect from WiFi"):
            response = api_post(api_session, web_base_url, "/api/wifi/disconnect")

        with allure.step("Verify disconnect response"):
            response.assert_ok()
            response.assert_has_fields("result").attach_to_allure("WiFi Disconnect Response")
