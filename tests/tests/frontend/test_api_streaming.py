import allure
import pytest

from utils import api_get


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
            response = api_get(
                api_session, web_base_url, "/api/screen",
                params={"display": 0}
            )

        with allure.step("Verify screen response"):
            response.assert_ok()

            content_type = response.headers.get("content-type", "")
            assert (
                "image/bmp" in content_type.lower()
            ), f"Expected BMP image, got {content_type}"
            assert len(response.response.content) > 0, "Image data should not be empty"
            allure.attach(
                response.response.content, "Front Display Frame", allure.attachment_type.BMP
            )

    @allure.id("2668")
    @allure.title("GET /api/screen (back display)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_screen_back_display(self, api_session, web_base_url):
        """Test GET /api/screen endpoint for back display"""

        with allure.step("Get back display frame"):
            response = api_get(
                api_session, web_base_url, "/api/screen",
                params={"display": 1}
            )

        with allure.step("Verify screen response"):
            response.assert_ok()

            content_type = response.headers.get("content-type", "")
            assert (
                "image/bmp" in content_type.lower()
            ), f"Expected BMP image, got {content_type}"
            assert len(response.response.content) > 0, "Image data should not be empty"
            allure.attach(
                response.response.content, "Back Display Frame", allure.attachment_type.BMP
            )

    @allure.id("2669")
    @allure.title("GET /api/screen (invalid display)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_screen_invalid_display(self, api_session, web_base_url):
        """Test GET /api/screen endpoint with invalid display number"""

        with allure.step("Request invalid display"):
            response = api_get(
                api_session, web_base_url, "/api/screen",
                params={"display": 2}
            )

        with allure.step("Verify error response"):
            response.assert_bad_request()
