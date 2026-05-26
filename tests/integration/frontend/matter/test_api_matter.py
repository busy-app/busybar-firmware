import os
import time

import allure
import pytest

from clients.api import SmartHomeAPI


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
        response = smart_home_api.start_pairing()

        # May return 200 with payload or 503 if smart home service is unavailable
        assert response.status_code in [200, 503]

        if response.status_code == 200:
            data = response.json()
            assert "qr_code" in data
            assert "manual_code" in data
            assert "available_until" in data

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

    @allure.title("POST /api/smart_home/pairing exposes seconds-level timing")
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

        response = smart_home_api.start_pairing()
        if response.status_code == 503:
            pytest.skip("Smart home pairing service is unavailable")
        assert response.status_code == 200

        payload = response.json()
        assert payload["qr_code"]
        assert payload["manual_code"]
        assert payload["available_until"]
        if str(payload["available_until"]).isdigit():
            assert int(payload["available_until"]) < 10_000_000_000

        time.sleep(0.5)
        status = smart_home_api.get_pairing()
        timestamp = status.latest_pairing_status.timestamp
        assert isinstance(timestamp, int)
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
        response = smart_home_api.get_switch_state()

        assert isinstance(response.state, bool)

    @allure.title("POST /api/smart_home/switch (set state)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_smart_home_switch_set_state(self, smart_home_api: SmartHomeAPI):
        """Test POST /api/smart_home/switch sets switch state"""
        # Get original state
        original = smart_home_api.get_switch_state()

        try:
            with allure.step("Set switch state to True"):
                response = smart_home_api.set_switch_state(True)
                assert response.status_code in [200, 503]

            if response.status_code == 200:
                with allure.step("Verify state was set"):
                    updated = smart_home_api.get_switch_state()
                    assert updated.state is True

                with allure.step("Set switch state to False"):
                    response = smart_home_api.set_switch_state(False)
                    assert response.status_code == 200

                with allure.step("Verify state was toggled"):
                    updated = smart_home_api.get_switch_state()
                    assert updated.state is False
        finally:
            with allure.step("Restore original state"):
                smart_home_api.set_switch_state(original.state)
