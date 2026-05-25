import allure
import pytest

from clients.api import StreamingAPI
from utils.simple_websocket import websocket_upgrade, websocket_url


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


@allure.feature("5. Web Frontend")
@allure.story("Streaming")
@pytest.mark.api
@pytest.mark.frontend
@pytest.mark.regression
class TestStreamingWebSocketRegressions:
    @allure.title("Removed /api/screen/ws does not upgrade")
    def test_removed_screen_websocket_api_does_not_upgrade(self, web_base_url):
        result = websocket_upgrade(websocket_url(web_base_url, "/api/screen/ws?display=0"))

        assert result.status_code != 101
        assert result.status_code in {400, 404, 405}

    @allure.title("Removed /api/screen/ws plain HTTP endpoint is not available")
    def test_removed_screen_websocket_plain_http_contract(self, api_session, web_base_url):
        response = api_session.get(f"{web_base_url}/api/screen/ws", timeout=10)

        assert response.status_code != 101
        assert response.status_code in {400, 404, 405}
        assert "upgrade" not in response.headers.get("Connection", "").lower()
