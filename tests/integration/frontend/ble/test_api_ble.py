import time

import allure
import pytest

from clients.api import BleAPI


def _wait_for_ble_status(
    ble_api: BleAPI, expected: str, timeout: float = 3.0, interval: float = 0.25
) -> str:
    """Poll /api/ble/status until it equals `expected` or the timeout expires.

    Returns the final observed status (so a failed assertion shows the actual
    state). BLE goes through `reset`/`initialization` after enable() and after
    a reboot before settling on `enabled`/`disabled`; reading it immediately
    is racy.
    """
    deadline = time.monotonic() + timeout
    last = ble_api.get_status().status
    while last != expected and time.monotonic() < deadline:
        time.sleep(interval)
        last = ble_api.get_status().status
    return last


@allure.feature("5. Web Frontend")
@allure.story("BLE")
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
    @pytest.mark.timeout(900)
    def test_api_ble_preserve_status_over_reboot(
        self, ble_api: BleAPI, device_flasher
    ):
        """Test that BLE enabled/disabled status is preserved over reboot"""
        # Toggle BLE and verify the persisted state survives a hard reboot.
        # `_wait_for_ble_status` absorbs the transient `reset`/`initialization`
        # window after enable()/disable() and after the reboot. The reset
        # timeout is generous because reset_device() may block on the OpenOCD
        # lock for up to ~120s when a concurrent crash trace is running on
        # the runner — see device_flasher.reset_device.
        for action, expected in (("enable", "enabled"),
                                 ("disable", "disabled"),
                                 ("enable", "enabled")):
            getattr(ble_api, action)()
            assert _wait_for_ble_status(ble_api, expected) == expected
            assert device_flasher.reset_and_wait(wait_timeout=180.0), (
                "reset_and_wait failed"
            )
            assert _wait_for_ble_status(ble_api, expected) == expected

@allure.feature("5. Web Frontend")
@allure.story("BLE")
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
