import time

import allure
import pytest

from clients.api import InputAPI
from clients.state_pb import input_pb2
from clients.state_publisher import StatePublisherWebSocket, state_update_kinds
from utils.simple_websocket import websocket_upgrade, websocket_url
from utils.input_helpers import wait_for_switch_position


@allure.feature("5. Web Frontend")
@allure.story("State Publisher")
@pytest.mark.api
@pytest.mark.frontend
class TestStatePublisherRegressions:

    @allure.title("/api/status/ws upgrades to WebSocket (routing contract)")
    def test_status_ws_upgrade_contract(self, web_base_url):
        result = websocket_upgrade(websocket_url(web_base_url, "/api/status/ws"))

        assert result.status_code == 101

    @allure.title("/api/status/ws enable sends initial snapshot with frame")
    def test_status_ws_enable_emits_frame_update(self, web_base_url):
        with StatePublisherWebSocket(web_base_url) as state_ws:
            state_ws.enable()

            deadline = time.monotonic() + 4.0
            seen_update_kinds: set[str] = set()
            while time.monotonic() < deadline:
                state = state_ws.read_state(
                    timeout=max(0.1, deadline - time.monotonic())
                )
                for update in state.updates:
                    seen_update_kinds.update(state_update_kinds(update))
                if "frame" in seen_update_kinds:
                    break

        assert "frame" in seen_update_kinds

    @allure.title("/api/status/ws plain HTTP endpoint does not upgrade")
    def test_status_ws_plain_http_contract(self, api_session, web_base_url):
        response = api_session.get(f"{web_base_url}/api/status/ws", timeout=10)

        assert response.status_code == 405
        assert "GET" in response.headers.get("Allow", "")
        assert "upgrade" not in response.headers.get("Connection", "").lower()

    @allure.title("/api/status/ws streams decodable BSB_State protobuf frames")
    def test_status_ws_stream_decodes_protobuf_frame(self, web_base_url):
        with StatePublisherWebSocket(web_base_url) as state_ws:
            state_ws.enable()
            state = state_ws.read_state()

        assert state.timestamp is None or state.timestamp > 0
        assert len(state.updates) >= 0

    @allure.title("/api/status/ws publishes input updates as protobuf state")
    def test_status_ws_stream_publishes_input_update(
        self, web_base_url, input_api: InputAPI
    ):
        with StatePublisherWebSocket(web_base_url) as state_ws:
            state_ws.enable()
            input_response = input_api.send_key("back")
            assert input_response.status_code == 200

            deadline = time.monotonic() + 6.0
            seen_update_kinds: set[str] = set()
            while time.monotonic() < deadline:
                state = state_ws.read_state(timeout=max(0.1, deadline - time.monotonic()))
                for update in state.updates:
                    seen_update_kinds.update(state_update_kinds(update))
                if "input" in seen_update_kinds:
                    break

        assert "input" in seen_update_kinds

    @allure.title("/api/status/ws send=all snapshot includes the switch position")
    def test_status_ws_send_all_includes_switch_position(
        self, web_base_url, input_api: InputAPI
    ):
        assert input_pb2 is not None, "generated protobuf bindings are required"
        initial_position = input_api.get_switch().position

        try:
            with allure.step("Inject switch position: apps"):
                assert input_api.send_key("apps").status_code == 200
                wait_for_switch_position(input_api, "apps")

            with StatePublisherWebSocket(web_base_url) as state_ws:
                state_ws.enable()
                state_ws.send_all()

                deadline = time.monotonic() + 6.0
                positions: list[int] = []
                while time.monotonic() < deadline and not positions:
                    state = state_ws.read_state(
                        timeout=max(0.1, deadline - time.monotonic())
                    )
                    positions = [
                        update.payload.switch_event.position
                        for update in state.updates
                        if update.kind == "input"
                        and update.payload.WhichOneof("event") == "switch_event"
                    ]

            with allure.step("Verify the snapshot carries the injected position"):
                assert positions == [input_pb2.SwitchPosition.Value("APPS")]
        finally:
            if initial_position in InputAPI.SWITCH_POSITIONS:
                input_api.send_key(initial_position)
                wait_for_switch_position(input_api, initial_position)
