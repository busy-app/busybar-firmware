import allure
import pytest


@allure.feature("5. Web Frontend")
@allure.story("Streaming")
class TestStreamingAPI:
    """Test cases for Streaming API endpoints"""

    @allure.id("2667")
    @allure.title("GET /api/screen (front display)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_screen_front_display(self, api_session, web_base_url):
        """Test GET /api/screen endpoint for front display"""

        with allure.step("Get front display frame"):
            params = {"display": 0}  # Front display
            response = api_session.get(
                f"{web_base_url}/api/screen", params=params, timeout=10
            )

        with allure.step("Verify screen response"):
            assert response.status_code in [
                200,
            ], f"Expected 200, got {response.status_code}"

            if response.status_code == 200:
                content_type = response.headers.get("content-type", "")
                assert (
                    "image/bmp" in content_type.lower()
                ), f"Expected BMP image, got {content_type}"
                assert len(response.content) > 0, "Image data should not be empty"
                allure.attach(
                    response.content, "Front Display Frame", allure.attachment_type.BMP
                )

    @allure.id("2668")
    @allure.title("GET /api/screen (back display)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_screen_back_display(self, api_session, web_base_url):
        """Test GET /api/screen endpoint for back display"""

        with allure.step("Get back display frame"):
            params = {"display": 1}  # Back display
            response = api_session.get(
                f"{web_base_url}/api/screen", params=params, timeout=10
            )

        with allure.step("Verify screen response"):
            assert response.status_code in [
                200,
            ], f"Expected 200, got {response.status_code}"

            if response.status_code == 200:
                content_type = response.headers.get("content-type", "")
                assert (
                    "image/bmp" in content_type.lower()
                ), f"Expected BMP image, got {content_type}"
                assert len(response.content) > 0, "Image data should not be empty"
                allure.attach(
                    response.content, "Back Display Frame", allure.attachment_type.BMP
                )

    @allure.id("2669")
    @allure.title("GET /api/screen (invalid display)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_screen_invalid_display(self, api_session, web_base_url):
        """Test GET /api/screen endpoint with invalid display number"""

        with allure.step("Request invalid display"):
            params = {"display": 2}  # Invalid display number
            response = api_session.get(
                f"{web_base_url}/api/screen", params=params, timeout=10
            )

        with allure.step("Verify error response"):
            assert (
                response.status_code == 400
            ), f"Expected 400 for invalid display, got {response.status_code}"
