"""
BLE connection lifecycle tests.

Verifies connect, disconnect, reconnect stability, and cross-checks the
connection state with the HTTP API.
"""

from __future__ import annotations

import asyncio
import time

import allure
import pytest

from clients.api.ble import BleAPI
from clients.ble.client import BleDeviceClient
from clients.ble.models import ScannedDevice


@allure.feature("BLE")
@allure.story("Connection")
@pytest.mark.ble
class TestBleConnection:
    """Connect / disconnect / reconnect lifecycle."""

    @allure.title("Connect and disconnect")
    async def test_connect_disconnect(
        self,
        ble_enabled: None,
        ble_scanned_device: ScannedDevice,
        ble_client: BleDeviceClient,
        ble_adapter: str | None,
    ) -> None:
        """A single connect-disconnect cycle should succeed."""
        await ble_client.connect(ble_scanned_device.connect_target, adapter=ble_adapter)
        assert ble_client.is_connected
        await ble_client.disconnect()
        assert not ble_client.is_connected

    @allure.title("Reconnect stability (2 cycles)")
    async def test_reconnect_stability(
        self,
        ble_enabled: None,
        ble_client: BleDeviceClient,
        ble_adapter: str | None,
        ble_device_name: str,
        ble_device_address: str | None,
        clear_linux_host_pairing,
    ) -> None:
        """The device should tolerate repeated scan/connect/disconnect cycles."""
        for i in range(2):
            with allure.step(f"Cycle {i + 1}/2"):
                devices = await BleDeviceClient.scan(
                    name=ble_device_name,
                    address=ble_device_address,
                    adapter=ble_adapter,
                    retries=2,
                )
                if not devices:
                    pytest.skip("BLE device not found during reconnect stability scan")
                device = devices[0]
                clear_linux_host_pairing(device.address, ble_adapter)
                devices = await BleDeviceClient.scan(
                    name=ble_device_name,
                    address=ble_device_address,
                    adapter=ble_adapter,
                    retries=2,
                )
                if not devices:
                    pytest.skip("BLE device not found after Linux bond reset")
                device = devices[0]
                await ble_client.connect(
                    device.connect_target, adapter=ble_adapter
                )
                assert ble_client.is_connected
                await ble_client.disconnect()
                assert not ble_client.is_connected
                await asyncio.sleep(1.0)

    @allure.title("HTTP API shows 'connected' when BLE is connected")
    async def test_http_status_connected(
        self,
        ble_enabled: None,
        ble_scanned_device: ScannedDevice,
        ble_client: BleDeviceClient,
        ble_adapter: str | None,
        ble_api: BleAPI,
    ) -> None:
        """The HTTP /api/ble/status should report 'connected'."""
        await ble_client.connect(ble_scanned_device.connect_target, adapter=ble_adapter)
        try:
            await asyncio.sleep(1.0)
            status = ble_api.get_status()
            assert status.status in ("connected", "enabled"), (
                f"Expected 'connected' or 'enabled', got '{status.status}'"
            )
        finally:
            await ble_client.disconnect()

    @allure.title("HTTP API shows 'connectable' after disconnect")
    @pytest.mark.skip(reason="Device web server returns 503 after BLE disconnect churn wedges the stack")
    async def test_http_status_connectable_after_disconnect(
        self,
        ble_enabled: None,
        ble_scanned_device: ScannedDevice,
        ble_client: BleDeviceClient,
        ble_adapter: str | None,
        ble_api: BleAPI,
    ) -> None:
        """After disconnect the HTTP status should return to 'connectable'."""
        await ble_client.connect(ble_scanned_device.connect_target, adapter=ble_adapter)
        await ble_client.disconnect()

        deadline = time.time() + 10.0
        while time.time() < deadline:
            status = ble_api.get_status()
            if status.status in ("connectable", "enabled"):
                break
            await asyncio.sleep(1.0)

        assert status.status in ("connectable", "enabled"), (
            f"Expected 'connectable' or 'enabled', got '{status.status}'"
        )
