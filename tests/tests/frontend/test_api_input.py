import json

import allure
import pytest


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
            "up",
            "down",
            "ok",
            "back",
            "start",
            "busy",
            "off",
            "custom",
            "apps",
            "settings",
        ]

        for key in valid_keys:
            with allure.step(f"Send key event: {key}"):
                params = {"key": key}
                response = api_session.post(
                    f"{web_base_url}/api/input", params=params, timeout=10
                )

                assert response.status_code in [
                    200,
                ], f"Expected 200 for key {key}, got {response.status_code}"

                if response.status_code == 200:
                    response_data = response.json()
                    allure.attach(
                        json.dumps(response_data, indent=2),
                        f"Input {key} Response",
                        allure.attachment_type.JSON,
                    )

    @allure.id("2666")
    @allure.title("POST /api/input (invalid key)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_input_invalid_key(self, api_session, web_base_url):
        """Test POST /api/input endpoint with invalid key"""

        with allure.step("Send invalid key event"):
            params = {"key": "invalid_key"}
            response = api_session.post(
                f"{web_base_url}/api/input", params=params, timeout=10
            )

        with allure.step("Verify invalid key response"):
            assert (
                response.status_code == 400
            ), f"Expected 400 for invalid key, got {response.status_code}"

            error_data = response.json()
            allure.attach(
                json.dumps(error_data, indent=2),
                "Invalid Key Error Response",
                allure.attachment_type.JSON,
            )
            assert "error" in error_data, "Error response should contain 'error' field"
