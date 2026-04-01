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
