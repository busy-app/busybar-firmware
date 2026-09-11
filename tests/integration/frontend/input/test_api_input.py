import allure
import pytest

from clients.api import InputAPI
from utils.simple_websocket import websocket_upgrade, websocket_url
from utils.input_helpers import wait_for_switch_position


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

    @allure.title("GET /api/input/switch (tracks injected switch positions)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_input_switch_get(self, input_api: InputAPI):
        """GET /api/input/switch reports the switch position selected via POST /api/input"""
        initial_position = input_api.get_switch().position

        try:
            for position in InputAPI.SWITCH_POSITIONS:
                with allure.step(f"Inject switch position: {position}"):
                    assert input_api.send_key(position).status_code == 200

                with allure.step(f"Verify GET /api/input/switch reports {position}"):
                    wait_for_switch_position(input_api, position)
        finally:
            if initial_position in InputAPI.SWITCH_POSITIONS:
                input_api.send_key(initial_position)
                wait_for_switch_position(input_api, initial_position)


@allure.feature("5. Web Frontend")
@allure.story("Input")
@pytest.mark.api
@pytest.mark.frontend
@pytest.mark.regression
class TestInputWebSocketRegressions:
    @allure.title("Removed /api/input/ws does not upgrade")
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
