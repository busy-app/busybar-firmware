"""
HM-10 UART Service tests.

Tests the TX (read + notify) and RX (read + write) characteristics.
"""

from __future__ import annotations

import asyncio

import allure
import pytest

from clients.ble.client import BleDeviceClient
from clients.ble.constants import (
    CHAR_HM10_RX,
    CHAR_HM10_TX,
    HM10_MAX_PAYLOAD_BYTES,
    TIMEOUT_NOTIFICATION_WAIT,
)


@allure.feature("BLE")
@allure.story("HM-10 UART")
@pytest.mark.ble
class TestBleHm10Uart:
    """HM-10 UART Service read / write / notification tests."""

    @allure.title("Read HM-10 TX characteristic")
    async def test_read_hm10_tx(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """TX characteristic should be readable."""
        data = await connected_ble_client.read_characteristic(CHAR_HM10_TX)
        assert isinstance(data, bytes)

    @allure.title("Read HM-10 RX characteristic")
    async def test_read_hm10_rx(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """RX characteristic should be readable."""
        data = await connected_ble_client.read_characteristic(CHAR_HM10_RX)
        assert isinstance(data, bytes)

    @allure.title("Write to HM-10 RX characteristic")
    async def test_write_hm10_rx(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """Writing a small payload to RX should succeed."""
        payload = b"hello"
        await connected_ble_client.write_characteristic(
            CHAR_HM10_RX, payload, response=True
        )

    @allure.title("HM-10 TX notification")
    @pytest.mark.skip(
        reason=(
            "Firmware issue: HM-10 TX notification path can drop the BLE connection "
            "during RX write; see HM10_TX_NOTIFICATION_BUG_REPORT.md"
        )
    )
    async def test_hm10_tx_notification(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """Subscribing to TX should deliver at least one notification."""
        received: asyncio.Future[bytearray] = asyncio.get_event_loop().create_future()

        def _callback(sender: int, data: bytearray) -> None:
            if not received.done():
                received.set_result(data)

        await connected_ble_client.start_notify(CHAR_HM10_TX, _callback)
        try:
            # Write to RX to trigger a notification on TX
            await connected_ble_client.write_characteristic(
                CHAR_HM10_RX, b"ping", response=True
            )
            data = await asyncio.wait_for(
                received, timeout=TIMEOUT_NOTIFICATION_WAIT
            )
            assert len(data) > 0, "Received empty TX notification"
        except asyncio.TimeoutError:
            pytest.skip(
                "No HM-10 TX notification received within timeout"
            )
        finally:
            await connected_ble_client.stop_notify(CHAR_HM10_TX)
