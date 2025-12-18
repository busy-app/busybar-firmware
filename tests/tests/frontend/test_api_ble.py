import allure
import pytest

from utils import api_get, api_post, api_delete, assert_field_in


@allure.feature("5. Web Frontend")
@allure.story("BLE")
class TestBleAPI:
    """Test cases for BLE API endpoints"""

    @allure.id("2663")
    @allure.title("POST /api/ble/enable")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_ble_enable(self, api_session, web_base_url):
        """Test POST /api/ble/enable endpoint"""

        with allure.step("Enable BLE"):
            response = api_post(api_session, web_base_url, "/api/ble/enable")

        with allure.step("Verify enable response"):
            response.assert_ok()
            response.attach_to_allure("BLE Enable Response")

    @allure.id("2664")
    @allure.title("POST /api/ble/disable")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_ble_disable(self, api_session, web_base_url):
        """Test POST /api/ble/disable endpoint"""

        with allure.step("Disable BLE"):
            response = api_post(api_session, web_base_url, "/api/ble/disable")

        with allure.step("Verify disable response"):
            response.assert_ok()
            response.attach_to_allure("BLE Disable Response")


@allure.feature("5. Web Frontend")
@allure.story("BLE")
class TestBleStatusAPI:
    """Test cases for BLE Status API endpoints"""

    @allure.id("2724")
    @allure.title("GET /api/ble/status")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_ble_status_get(self, api_session, web_base_url):
        """Test GET /api/ble/status endpoint"""

        with allure.step("Make GET request to /api/ble/status"):
            response = api_get(api_session, web_base_url, "/api/ble/status")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("state", "pairing").attach_to_allure("BLE Status Response")

            data = response.json()

            # Validate state enum
            valid_states = [
                "reset", "initialization", "disabled",
                "enabled", "connected", "internal error",
            ]
            assert_field_in(data, "state", valid_states)

            # Address field is only present when BLE is enabled
            if data["state"] in ["enabled", "connected"]:
                response.assert_has_fields("address")

            # Validate pairing enum
            assert_field_in(data, "pairing", ["unknown", "not paired", "paired"])

    @allure.id("2725")
    @allure.title("DELETE /api/ble/pairing")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Destructive test - removes BLE pairing")
    def test_api_ble_pairing_delete(self, api_session, web_base_url):
        """Test DELETE /api/ble/pairing endpoint"""

        with allure.step("Remove BLE pairing"):
            response = api_delete(api_session, web_base_url, "/api/ble/pairing")

        with allure.step("Verify response status"):
            # May return 200 if successful, 503 if BLE not initialized or already unpaired
            response.assert_status([200, 503])
            response.attach_to_allure("BLE Pairing Remove Response")
