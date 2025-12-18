import allure
import pytest

from utils import api_post, attach_json, assert_has_fields


@allure.feature("5. Web Frontend")
@allure.story("Input")
class TestInputAPI:
    """Test cases for Input API endpoints"""

    @allure.id("2665")
    @allure.title("POST /api/input (key events)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_input_key_post(self, api_session, web_base_url):
        """Test POST /api/input endpoint for key events"""

        valid_keys = [
            "up", "down", "ok", "back", "start",
            "busy", "off", "custom", "apps", "settings",
        ]

        for key in valid_keys:
            with allure.step(f"Send key event: {key}"):
                response = api_post(
                    api_session, web_base_url, "/api/input",
                    params={"key": key}
                )
                response.assert_ok()
                response.attach_to_allure(f"Input {key} Response")

    @allure.id("2666")
    @allure.title("POST /api/input (invalid key)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_input_invalid_key(self, api_session, web_base_url):
        """Test POST /api/input endpoint with invalid key"""

        with allure.step("Send invalid key event"):
            response = api_post(
                api_session, web_base_url, "/api/input",
                params={"key": "invalid_key"}
            )

        with allure.step("Verify invalid key response"):
            response.assert_bad_request()

            data = response.json()
            response.attach_to_allure("Invalid Key Error Response")
            assert_has_fields(data, "error")
