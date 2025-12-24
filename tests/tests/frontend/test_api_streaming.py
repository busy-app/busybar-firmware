import allure
import pytest

from api import StreamingAPI


@allure.feature("5. Web Frontend")
@allure.story("Streaming")
class TestStreamingAPI:
    """Test cases for Streaming API endpoints"""

    @allure.id("2667")
    @allure.title("GET /api/screen (front display)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_screen_front_display(self, streaming_api: StreamingAPI):
        """Test GET /api/screen endpoint for front display"""
        response = streaming_api.get_front_display()

        assert response.status_code == 200

        content_type = response.headers.get("content-type", "")
        assert "image/bmp" in content_type.lower(), f"Expected BMP image, got {content_type}"
        assert len(response.content) > 0, "Image data should not be empty"

        allure.attach(response.content, "Front Display Frame", allure.attachment_type.BMP)

    @allure.id("2668")
    @allure.title("GET /api/screen (back display)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_screen_back_display(self, streaming_api: StreamingAPI):
        """Test GET /api/screen endpoint for back display"""
        response = streaming_api.get_back_display()

        assert response.status_code == 200

        content_type = response.headers.get("content-type", "")
        assert "image/bmp" in content_type.lower(), f"Expected BMP image, got {content_type}"
        assert len(response.content) > 0, "Image data should not be empty"

        allure.attach(response.content, "Back Display Frame", allure.attachment_type.BMP)

    @allure.id("2669")
    @allure.title("GET /api/screen (invalid display)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_screen_invalid_display(self, streaming_api: StreamingAPI):
        """Test GET /api/screen endpoint with invalid display number"""
        response = streaming_api.get_screen(display=2)

        assert response.status_code == 400
