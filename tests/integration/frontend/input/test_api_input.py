import allure
import pytest

from clients.api import InputAPI
from utils.simple_websocket import websocket_upgrade, websocket_url


pytestmark = [
    pytest.mark.bsb_metadata(
        suite="Integration",
        level="Integration",
        intent="Product",
        interfaces=("Device API",),
        transports=("LAN REST",),
    ),
]


@allure.epic("BusyBar & Local Control")
@allure.feature("Device HTTP API")
@allure.story("Input")
class TestInputAPI:
    """Test cases for Input API endpoints"""

    @allure.title("POST /api/input (key events)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_input_key_post(self, input_api: InputAPI):
        """Test POST /api/input endpoint for key events"""
        for key in InputAPI.VALID_KEYS:
            with allure.step(f"Send key event: {key}"):
                response = input_api.send_key(key)
                assert response.status_code == 200

    @allure.title("POST /api/input (invalid key)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_input_invalid_key(self, input_api: InputAPI):
        """Test POST /api/input endpoint with invalid key"""
        response = input_api.send_key("invalid_key")

        assert response.status_code == 400

        data = response.json()
        assert "error" in data


@allure.epic("BusyBar & Local Control")
@allure.feature("Device HTTP API")
@allure.story("Input")
@pytest.mark.api
@pytest.mark.frontend
@pytest.mark.regression
class TestInputWebSocketRegressions:
    @allure.title("Removed /api/input/ws does not upgrade")
    @pytest.mark.bsb_metadata(
        intent="Reliability",
        interfaces=("Device API", "WebSocket"),
        transports=("LAN REST", "WebSocket"),
    )
    def test_removed_input_websocket_api_does_not_upgrade(self, web_base_url):
        result = websocket_upgrade(websocket_url(web_base_url, "/api/input/ws"))

        assert result.status_code == 405
        assert "POST" in result.headers.get("allow", "")

    @allure.title("Removed /api/input/ws plain HTTP endpoint is not available")
    def test_removed_input_websocket_plain_http_contract(self, api_session, web_base_url):
        response = api_session.get(f"{web_base_url}/api/input/ws", timeout=10)

        assert response.status_code == 405
        assert "POST" in response.headers.get("Allow", "")
        assert "upgrade" not in response.headers.get("Connection", "").lower()
