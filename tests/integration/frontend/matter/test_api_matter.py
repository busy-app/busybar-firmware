import allure
import pytest

from clients.api import MatterAPI


@allure.feature("5. Web Frontend")
@allure.story("Matter")
class TestMatterCommissioningAPI:
    """Test cases for Matter Commissioning API endpoints"""

    @allure.title("GET /api/matter/commissioning")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_matter_commissioning_get(self, matter_api: MatterAPI):
        """Test GET /api/matter/commissioning returns commissioning status"""
        response = matter_api.get_commissioning()

        assert response.fabric_count >= 0
        assert response.latest_commissioning_status is not None
        assert response.latest_commissioning_status.value in [
            "never_started", "started", "completed_successfully", "failed"
        ]

    @allure.title("POST /api/matter/commissioning (start)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_matter_commissioning_start(self, matter_api: MatterAPI):
        """Test POST /api/matter/commissioning starts commissioning"""
        response = matter_api.start_commissioning()

        # May return 200 with payload or 503 if Matter service is unavailable
        assert response.status_code in [200, 503]

        if response.status_code == 200:
            data = response.json()
            assert "qr_code" in data
            assert "manual_code" in data
            assert "available_until" in data


@allure.feature("5. Web Frontend")
@allure.story("Matter")
class TestMatterEndpointAPI:
    """Test cases for Matter Endpoint API"""

    @allure.title("GET /api/matter/endpoint/1")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_matter_endpoint_get(self, matter_api: MatterAPI):
        """Test GET /api/matter/endpoint/1 returns endpoint state"""
        response = matter_api.get_endpoint_state()

        assert response.type == "switch"
        assert isinstance(response.state, bool)

    @allure.title("POST /api/matter/endpoint/1 (set state)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_matter_endpoint_set_state(self, matter_api: MatterAPI):
        """Test POST /api/matter/endpoint/1 sets endpoint state"""
        # Get original state
        original = matter_api.get_endpoint_state()

        try:
            with allure.step("Set endpoint state to True"):
                response = matter_api.set_endpoint_state(True)
                assert response.status_code in [200, 503]

            if response.status_code == 200:
                with allure.step("Verify state was set"):
                    updated = matter_api.get_endpoint_state()
                    assert updated.state is True

                with allure.step("Set endpoint state to False"):
                    response = matter_api.set_endpoint_state(False)
                    assert response.status_code == 200

                with allure.step("Verify state was toggled"):
                    updated = matter_api.get_endpoint_state()
                    assert updated.state is False
        finally:
            with allure.step("Restore original state"):
                matter_api.set_endpoint_state(original.state)
