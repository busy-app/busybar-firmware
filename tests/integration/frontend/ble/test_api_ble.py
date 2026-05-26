import time

import allure
import pytest

from clients.api import BleAPI


BLE_ENABLED_STATES = frozenset({"enabled", "connectable", "connected"})
BLE_DISABLED_STATES = frozenset({"disabled"})


def _wait_for_ble_status(
    ble_api: BleAPI,
    expected,
    timeout: float = 3.0,
    interval: float = 0.25,
) -> str:
    """Poll /api/ble/status until it lands in `expected` (str or set) or times out."""
    expected_set = {expected} if isinstance(expected, str) else set(expected)
    deadline = time.monotonic() + timeout
    last = ""
    while time.monotonic() < deadline:
        try:
            last = ble_api.get_status().status
            if last in expected_set:
                return last
        except Exception as exc:
            last = f"<{type(exc).__name__}: {exc}>"
        time.sleep(interval)
    return last


@allure.feature("5. Web Frontend")
@allure.story("BLE")
@pytest.mark.uses_si917
class TestBleAPI:
    """Test cases for BLE API endpoints"""

    @allure.id("2663")
    @allure.title("POST /api/ble/enable")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_ble_enable(self, ble_api: BleAPI):
        """Test POST /api/ble/enable endpoint"""
        response = ble_api.enable()

        assert response.status_code == 200

    @allure.id("2664")
    @allure.title("POST /api/ble/disable")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_ble_disable(self, ble_api: BleAPI):
        """Test POST /api/ble/disable endpoint"""
        response = ble_api.disable()

        assert response.status_code == 200

    @allure.id("3597")
    @allure.title("#3564 BLE. Preserve status over reboot")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.regression
    @pytest.mark.timeout(300)
    def test_api_ble_preserve_status_over_reboot(
        self, ble_api: BleAPI, persistent_cli_connection, web_base_url
    ):
        """Test that BLE enabled/disabled status is preserved over reboot"""
        for action, expected_set in (("enable",  BLE_ENABLED_STATES),
                                     ("disable", BLE_DISABLED_STATES),
                                     ("enable",  BLE_ENABLED_STATES)):
            getattr(ble_api, action)()
            last = _wait_for_ble_status(ble_api, expected_set)
            assert last in expected_set, f"pre-reboot: got {last!r}, expected one of {sorted(expected_set)}"
            assert persistent_cli_connection.reboot_and_wait_for_api(
                web_base_url
            ), "device did not come back after CLI reboot"
            last = _wait_for_ble_status(ble_api, expected_set, timeout=30.0)
            assert last in expected_set, f"post-reboot: got {last!r}, expected one of {sorted(expected_set)}"

@allure.feature("5. Web Frontend")
@allure.story("BLE")
@pytest.mark.uses_si917
class TestBleStatusAPI:
    """Test cases for BLE Status API endpoints"""

    @allure.id("3870")
    @allure.title("GET /api/ble/status")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_ble_status_get(self, ble_api: BleAPI):
        """Test GET /api/ble/status endpoint"""
        response = ble_api.get_status()

        # Status enum validated by pydantic
        valid_states = [
            "reset", "initialization", "disabled",
            "enabled", "connectable", "connected", "internal error",
        ]
        assert response.status in valid_states

        # Address field may be present when BLE is active
        if response.status == "connected":
            assert response.address is not None

    @allure.id("3871")
    @allure.title("DELETE /api/ble/pairing")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_ble_pairing_delete(self, ble_api: BleAPI):
        """Test DELETE /api/ble/pairing endpoint"""
        response = ble_api.remove_pairing()

        # May return 200 if successful, 503 if BLE not initialized or already unpaired
        assert response.status_code in [200, 503]
