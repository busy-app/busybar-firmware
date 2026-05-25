from __future__ import annotations

import pytest


class BleStateTransport:
    name = "ble"

    def __init__(self, *args, **kwargs):
        self.args = args
        self.kwargs = kwargs

    def connect(self) -> None:
        pytest.skip("BLE state publisher transport is not wired to a real BLE client yet")

    def enable(self) -> None:
        pytest.skip("BLE state publisher transport is not wired to a real BLE client yet")

    def read_frame(self, timeout: float = 6.0) -> bytes:
        pytest.skip("BLE state publisher transport is not wired to a real BLE client yet")

    def read_state(self, timeout: float = 6.0):
        pytest.skip("BLE state publisher transport is not wired to a real BLE client yet")

    def close(self) -> None:
        return None
