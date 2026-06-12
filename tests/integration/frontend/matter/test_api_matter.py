import os
import time

import allure
import pytest
import requests

from clients.api import SmartHomeAPI

from clients.api.base import APIError

# Retry settings for non-blocking Matter API operations.
# After the queue-based refactor (PR #814), set_switch_state returns
# immediately after queuing; the cached FuriState is updated
# asynchronously by unsolicited Si917 responses.
# After a device reboot the Si917 may take 10–15 s to start reporting
# state, so allow a generous polling window.
_STATE_POLL_RETRIES = 15
_STATE_POLL_DELAY_S = 0.25


def _wait_for_device(smart_home_api: SmartHomeAPI, timeout_s: float = 30) -> None:
    """Wait for the device to become reachable after a reboot."""
    # Use a fresh session to avoid reusing a stale TCP connection.
    session = requests.Session()
    try:
        deadline = time.monotonic() + timeout_s
        while time.monotonic() < deadline:
            try:
                resp = session.get(
                    f"{smart_home_api.base_url}/api/version", timeout=1
                )
                if resp.status_code == 200:
                    return
            except (requests.ConnectionError, requests.ReadTimeout):
                pass
            time.sleep(0.25)
    finally:
        session.close()
    pytest.fail(f"Device did not come back within {timeout_s}s")


def _get_switch_state_safe(smart_home_api: SmartHomeAPI) -> bool | None:
    """Get switch state, returning None on 503 or timeout (service unavailable)."""
    try:
        return smart_home_api.get_switch_state().state
    except APIError as e:
        if e.status_code == 503:
            return None
        raise
    except (requests.ReadTimeout, requests.ConnectionError):
        return None


def _poll_switch_state(
    smart_home_api: SmartHomeAPI, expected: bool
) -> bool:
    """Poll GET /switch until state matches expected value or timeout."""
    for _ in range(_STATE_POLL_RETRIES):
        state = _get_switch_state_safe(smart_home_api)
        if state is expected:
            return True
        time.sleep(_STATE_POLL_DELAY_S)
    return False


@allure.feature("5. Web Frontend")
@allure.story("Smart Home")
class TestSmartHomePairingAPI:
    """Test cases for Smart Home Pairing API endpoints"""

    @allure.title("GET /api/smart_home/pairing")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_smart_home_pairing_get(self, smart_home_api: SmartHomeAPI):
        """Test GET /api/smart_home/pairing returns pairing status"""
        response = smart_home_api.get_pairing()

        assert response.fabric_count >= 0
        assert response.latest_pairing_status is not None
        assert response.latest_pairing_status.value in [
            "never_started",
            "started",
            "completed_successfully",
            "failed",
        ]
        if response.latest_pairing_status.timestamp is not None:
            assert isinstance(response.latest_pairing_status.timestamp, int)

    @allure.title("POST /api/smart_home/pairing (start)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_smart_home_pairing_start(self, smart_home_api: SmartHomeAPI):
        """Test POST /api/smart_home/pairing starts pairing"""
        try:
            response = smart_home_api.start_pairing()
        except requests.ReadTimeout:
            pytest.skip("Smart home pairing service is not responding in time")

        # May return 200 with payload or 503 if smart home service is unavailable
        assert response.status_code in [200, 503]

        if response.status_code == 200:
            data = response.json()
            assert "qr_code" in data
            assert "manual_code" in data
            assert "available_until" in data

    @allure.title("DELETE /api/smart_home/pairing (factory reset)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_smart_home_pairing_delete(self, smart_home_api: SmartHomeAPI):
        """Test DELETE /api/smart_home/pairing resets pairing info"""
        response = smart_home_api.erase_pairing()

        assert response.result == "OK"

        # Factory reset triggers a deferred reboot (~2.5 s on device).
        # Wait for the device to come back so subsequent tests are not
        # affected.
        _wait_for_device(smart_home_api, timeout_s=30)

    @allure.title("GET /api/smart_home/pairing timestamp uses seconds resolution")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.regression
    def test_pairing_status_timestamp_is_seconds_resolution(
        self, smart_home_api: SmartHomeAPI
    ):
        status = smart_home_api.get_pairing()
        timestamp = status.latest_pairing_status.timestamp

        if timestamp is None:
            pytest.skip("Smart home pairing status has no timestamp yet")

        assert isinstance(timestamp, int)
        assert 0 < timestamp < 10_000_000_000, (
            "Expected Unix seconds. A millisecond timestamp would be much larger."
        )

    @allure.title("POST /api/smart_home/pairing exposes milliseconds-level timing")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.regression
    def test_pairing_start_timing_fields_are_seconds_level(
        self, smart_home_api: SmartHomeAPI
    ):
        if os.getenv("BSB_ENABLE_MATTER_PAIRING_REGRESSION", "").lower() not in {
            "1",
            "true",
            "yes",
        }:
            pytest.skip("Matter active pairing regression is disabled on this bench")

        try:
            response = smart_home_api.start_pairing()
        except requests.ReadTimeout:
            pytest.skip("Smart home pairing service is not responding in time")
        if response.status_code == 503:
            pytest.skip("Smart home pairing service is unavailable")
        assert response.status_code == 200

        payload = response.json()
        assert payload["qr_code"]
        assert payload["manual_code"]
        assert payload["available_until"]

        # available_until is a UTC Unix millisecond timestamp (as a string)
        available_ms = int(payload["available_until"])
        assert 1_700_000_000_000 < available_ms < 10_000_000_000_000, (
            f"Expected Unix ms. Got {available_ms}"
        )

        time.sleep(0.5)
        status = smart_home_api.get_pairing()
        timestamp = status.latest_pairing_status.timestamp
        assert isinstance(timestamp, int)
        # latest_pairing_status.timestamp is Unix seconds
        assert 0 < timestamp < 10_000_000_000


@allure.feature("5. Web Frontend")
@allure.story("Smart Home")
class TestSmartHomeSwitchAPI:
    """Test cases for Smart Home Switch API"""

    @allure.title("GET /api/smart_home/switch")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_smart_home_switch_get(self, smart_home_api: SmartHomeAPI):
        """Test GET /api/smart_home/switch returns switch state"""
        state = _get_switch_state_safe(smart_home_api)
        if state is None:
            pytest.skip("Smart home service is unavailable")

        assert isinstance(state, bool)

    @allure.title("POST /api/smart_home/switch (set state)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_smart_home_switch_set_state(self, smart_home_api: SmartHomeAPI):
        """Test POST /api/smart_home/switch sets switch state"""
        # After a reboot (e.g. triggered by DELETE /pairing), the Si917
        # backend may need time to start reporting state.  Wait for the
        # switch state to become known before toggling.
        if not _poll_switch_state(smart_home_api, True) and not _poll_switch_state(
            smart_home_api, False
        ):
            pytest.skip("Switch state never became known (Si917 may be offline)")

        original = _get_switch_state_safe(smart_home_api)
        target = not original

        try:
            with allure.step(f"Toggle switch state to {target}"):
                response = smart_home_api.set_switch_state(target)
                assert response.status_code in [200, 503]

            if response.status_code == 503:
                pytest.skip("Smart home service is unavailable")

            with allure.step(f"Wait for state to propagate to {target}"):
                assert _poll_switch_state(
                    smart_home_api, target
                ), f"Switch state did not become {target} within {_STATE_POLL_RETRIES * _STATE_POLL_DELAY_S:.0f}s"

            with allure.step(f"Toggle switch state back to {original}"):
                response = smart_home_api.set_switch_state(original)
                assert response.status_code == 200

            with allure.step(f"Wait for state to propagate back to {original}"):
                assert _poll_switch_state(
                    smart_home_api, original
                ), f"Switch state did not become {original} within {_STATE_POLL_RETRIES * _STATE_POLL_DELAY_S:.0f}s"
        finally:
            pass  # original was restored inside the block

    @allure.title("POST /api/smart_home/switch (set startup mode)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_smart_home_switch_set_startup(self, smart_home_api: SmartHomeAPI):
        """Test POST /api/smart_home/switch sets startup mode"""
        for mode in ("off", "on", "toggle", "last"):
            with allure.step(f"Set startup mode to '{mode}'"):
                response = smart_home_api.set_switch_startup(mode)
                assert response.status_code in [200, 503]
                if response.status_code == 503:
                    pytest.skip("Smart home service is unavailable")

    @allure.title("POST /api/smart_home/switch (set state and startup together)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_smart_home_switch_set_combined(self, smart_home_api: SmartHomeAPI):
        """Test POST /api/smart_home/switch with both state and startup"""
        response = smart_home_api.set_switch_state(True, startup="last")
        assert response.status_code in [200, 503]

        if response.status_code == 503:
            pytest.skip("Smart home service is unavailable")

        response = smart_home_api.set_switch_state(False, startup="off")
        assert response.status_code in [200, 503]

    @allure.title("POST /api/smart_home/switch returns 400 on empty body")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_smart_home_switch_empty_body_returns_400(
        self, smart_home_api: SmartHomeAPI
    ):
        """Test POST /api/smart_home/switch rejects empty request body"""
        response = smart_home_api.post_raw(
            "/api/smart_home/switch", data=b"{}"
        )
        assert response.status_code == 400
