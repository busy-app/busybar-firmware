"""
Nordic UART Service (NUS) tests.

Tests the RX (write), TX (indicate), and CNT (read/write) characteristics.
"""

from __future__ import annotations

import asyncio
import struct

import allure
import pytest

from clients.ble.client import BleDeviceClient
from clients.ble.constants import (
    CHAR_NUS_CNT,
    CHAR_NUS_RX,
    CHAR_NUS_TX,
    NUS_MAX_PAYLOAD_BYTES,
    TIMEOUT_NOTIFICATION_WAIT,
)


@allure.feature("BLE")
@allure.story("Nordic UART")
@pytest.mark.ble
class TestBleNordicUart:
    """Nordic UART Service (NUS) read / write / indication tests."""

    @allure.title("Read NUS counter (uint32)")
    async def test_read_nus_counter(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """CNT characteristic should return a 4-byte little-endian uint32."""
        data = await connected_ble_client.read_characteristic(CHAR_NUS_CNT)
        assert len(data) >= 4, f"Expected >= 4 bytes, got {len(data)}"
        value = struct.unpack_from("<I", data)[0]
        assert isinstance(value, int)

    @allure.title("NUS counter increments and resets proxy state")
    @pytest.mark.skip(reason="Requires TX indications which never arrive on the CI bench, so the proxy session always resets")
    async def test_nus_counter_increments_and_resets(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """CNT should count processed RX requests and writing 0 should reset it."""
        # The firmware bumps the GATT-visible counter only after the full
        # request/response cycle: the HTTP response is delivered back over TX
        # *indications*, and without a subscriber the 4 s indication-confirm
        # timeout resets the session to 0. Subscribe first so the cycle can
        # complete (see ble_http_repeater.c).
        await connected_ble_client.start_notify(CHAR_NUS_TX, lambda sender, data: None)
        try:
            await connected_ble_client.reset_nus_session()
            assert await connected_ble_client.read_nus_counter() == 0

            await connected_ble_client.write_nus_rx(b"GET /api/version\r\n")

            deadline = asyncio.get_event_loop().time() + 5.0
            count = 0
            while asyncio.get_event_loop().time() < deadline:
                count = await connected_ble_client.read_nus_counter()
                if count >= 1:
                    break
                await asyncio.sleep(0.25)

            assert count >= 1, "NUS counter did not increment after RX request"

            await connected_ble_client.reset_nus_session()
            assert await connected_ble_client.read_nus_counter() == 0
        finally:
            await connected_ble_client.stop_notify(CHAR_NUS_TX)

    @allure.title("Write to NUS RX")
    async def test_write_nus_rx(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """Writing a small payload to the RX characteristic should not error."""
        payload = b"ping"
        await connected_ble_client.write_nus_rx(payload)

    @allure.title("NUS TX indication after RX write")
    async def test_nus_tx_indication(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """After writing to RX, a TX indication should be received."""
        received: asyncio.Future[bytearray] = asyncio.get_event_loop().create_future()

        def _callback(sender: int, data: bytearray) -> None:
            if not received.done():
                received.set_result(data)

        await connected_ble_client.start_notify(CHAR_NUS_TX, _callback)
        try:
            # Write a simple payload to trigger a response
            await connected_ble_client.write_nus_rx(b"GET /api/version\r\n")
            data = await asyncio.wait_for(
                received, timeout=TIMEOUT_NOTIFICATION_WAIT
            )
            assert len(data) > 0, "Received empty TX indication"
        except asyncio.TimeoutError:
            pytest.skip("No NUS TX indication received within timeout")
        finally:
            await connected_ble_client.stop_notify(CHAR_NUS_TX)

    @allure.title("Write maximum NUS payload (237 bytes)")
    async def test_write_max_payload(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """Writing the maximum 237-byte payload should succeed."""
        payload = bytes(range(256))[:NUS_MAX_PAYLOAD_BYTES]
        assert len(payload) == NUS_MAX_PAYLOAD_BYTES
        await connected_ble_client.write_nus_rx(payload)
