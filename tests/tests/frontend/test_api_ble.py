import json

import allure
import pytest


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
            response = api_session.post(f"{web_base_url}/api/ble/enable", timeout=10)

        with allure.step("Verify enable response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "BLE Enable Response",
                allure.attachment_type.JSON,
            )

    @allure.id("2664")
    @allure.title("POST /api/ble/disable")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_ble_disable(self, api_session, web_base_url):
        """Test POST /api/ble/disable endpoint"""

        with allure.step("Disable BLE"):
            response = api_session.post(f"{web_base_url}/api/ble/disable", timeout=10)

        with allure.step("Verify disable response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "BLE Disable Response",
                allure.attachment_type.JSON,
            )


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
            response = api_session.get(f"{web_base_url}/api/ble/status", timeout=10)

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            ble_data = response.json()
            allure.attach(
                json.dumps(ble_data, indent=2),
                "BLE Status Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields based on OpenAPI schema
            assert "state" in ble_data, "Response should contain 'state' field"
            assert "pairing" in ble_data, "Response should contain 'pairing' field"

            # Validate state enum
            valid_states = [
                "reset",
                "initialization",
                "disabled",
                "enabled",
                "connected",
                "internal error",
            ]
            assert (
                ble_data["state"] in valid_states
            ), f"State should be one of {valid_states}, got {ble_data['state']}"

            # Address field is only present when BLE is enabled
            if ble_data["state"] in ["enabled", "connected"]:
                assert "address" in ble_data, "Response should contain 'address' field when BLE is enabled"

            # Validate pairing enum
            valid_pairing = ["unknown", "not paired", "paired"]
            assert (
                ble_data["pairing"] in valid_pairing
            ), f"Pairing should be one of {valid_pairing}, got {ble_data['pairing']}"

    @allure.id("2725")
    @allure.title("DELETE /api/ble/pairing")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Destructive test - removes BLE pairing")
    def test_api_ble_pairing_delete(self, api_session, web_base_url):
        """Test DELETE /api/ble/pairing endpoint"""

        with allure.step("Remove BLE pairing"):
            response = api_session.delete(f"{web_base_url}/api/ble/pairing", timeout=10)

        with allure.step("Verify response status"):
            # May return 200 if successful, 503 if BLE not initialized or already unpaired
            assert response.status_code in [
                200,
                503,
            ], f"Expected 200 or 503, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "BLE Pairing Remove Response",
                allure.attachment_type.JSON,
            )
