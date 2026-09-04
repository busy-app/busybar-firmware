from __future__ import annotations

import time

from clients.state_publisher.decoder import decode_state_frame
from clients.state_publisher.models import StateFrame
from utils.simple_websocket import SimpleWebSocket, websocket_url


class WsStateTransport:
    name = "ws"

    def __init__(self, web_base_url: str):
        self.web_base_url = web_base_url
        self.ws = SimpleWebSocket(websocket_url(web_base_url, "/api/status/ws"))

    def __enter__(self) -> "WsStateTransport":
        self.connect()
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    def connect(self) -> None:
        self.ws.connect()

    def enable(self) -> None:
        self.ws.send_text('{"enable":true}')

    def send_all(self) -> None:
        self.ws.send_text('{"send":"all"}')

    def read_frame(self, timeout: float = 6.0) -> bytes:
        deadline = time.monotonic() + timeout
        last_error: Exception | None = None
        while time.monotonic() < deadline:
            try:
                frame = self.ws.recv_data_frame(
                    timeout=max(0.1, deadline - time.monotonic())
                )
            except Exception as exc:
                last_error = exc
                break
            if frame.opcode == 0x2:
                return frame.payload
        raise AssertionError(f"No binary state protobuf frame received: {last_error}")

    def read_state(self, timeout: float = 6.0) -> StateFrame:
        return decode_state_frame(self.read_frame(timeout=timeout))

    def close(self) -> None:
        self.ws.close()


class StatePublisherWebSocket(WsStateTransport):
    pass
