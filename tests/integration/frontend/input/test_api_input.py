import allure
import pytest

from clients.api import InputAPI


@allure.feature("5. Web Frontend")
@allure.story("Input")
class TestInputAPI:
    """Test cases for Input API endpoints"""

    @allure.id("2665")
    @allure.title("POST /api/input (key events)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_input_key_post(self, input_api: InputAPI):
        """Test POST /api/input endpoint for key events"""
        for key in InputAPI.VALID_KEYS:
            with allure.step(f"Send key event: {key}"):
                response = input_api.send_key(key)
                assert response.status_code == 200

    @allure.id("2666")
    @allure.title("POST /api/input (invalid key)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_input_invalid_key(self, input_api: InputAPI):
        """Test POST /api/input endpoint with invalid key"""
        response = input_api.send_key("invalid_key")

        assert response.status_code == 400

        data = response.json()
        assert "error" in data
