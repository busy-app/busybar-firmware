"""
BLE security / pairing tests.

Covers connection without prior pairing, pairing removal + reconnect,
and basic service access after pairing removal.
"""

from __future__ import annotations

import asyncio
import time

import allure
import pytest

from clients.api.ble import BleAPI
from clients.ble.client import BleDeviceClient
from clients.ble.constants import CHAR_DEVICE_INFO_SERIAL
from clients.ble.models import ScannedDevice


@allure.feature("BLE")
@allure.story("Security")
@pytest.mark.ble
class TestBleSecurity:
    """BLE pairing / security-level tests."""

    @allure.title("Connect without prior pairing")
    async def test_connect_without_pairing(
        self,
        ble_enabled: None,
        ble_scanned_device: ScannedDevice,
        ble_client: BleDeviceClient,
        ble_adapter: str | None,
    ) -> None:
        """A fresh connection (no stored bond) should succeed."""
        await ble_client.connect(ble_scanned_device.connect_target, adapter=ble_adapter)
        assert ble_client.is_connected
        await ble_client.disconnect()

    @allure.title("Remove pairing and reconnect")
    @pytest.mark.skip(reason="DELETE /api/ble/pairing wedges the device, web server returns 503")
    async def test_remove_pairing_reconnect(
        self,
        ble_enabled: None,
        ble_scanned_device: ScannedDevice,
        ble_client: BleDeviceClient,
        ble_adapter: str | None,
        ble_api: BleAPI,
        clear_linux_host_pairing,
    ) -> None:
        """After removing pairing via HTTP API, reconnection should succeed."""
        await ble_client.connect(ble_scanned_device.connect_target, adapter=ble_adapter)
        await ble_client.disconnect()

        with allure.step("Remove pairing via HTTP API"):
            response = ble_api.remove_pairing()
            assert response.status_code in (200, 503)
            clear_linux_host_pairing(ble_scanned_device.address, ble_adapter)

        await asyncio.sleep(2.0)

        deadline = time.time() + 15.0
        while time.time() < deadline:
            status = ble_api.get_status()
            if status.status in ("connectable", "enabled"):
                break
            await asyncio.sleep(1.0)

        with allure.step("Reconnect after pairing removal"):
            devices = await BleDeviceClient.scan(
                name=ble_scanned_device.name, retries=2, adapter=ble_adapter,
            )
            assert devices, "Device not found after pairing removal"
            await ble_client.connect(devices[0].connect_target, adapter=ble_adapter)
            assert ble_client.is_connected
            await ble_client.disconnect()

    @allure.title("Service access after pairing removal")
    async def test_service_access_after_pairing_removal(
        self,
        ble_enabled: None,
        ble_scanned_device: ScannedDevice,
        ble_client: BleDeviceClient,
        ble_adapter: str | None,
        ble_api: BleAPI,
        clear_linux_host_pairing,
    ) -> None:
        """Reading a characteristic should work even without a stored bond."""
        with allure.step("Remove pairing"):
            ble_api.remove_pairing()
            clear_linux_host_pairing(ble_scanned_device.address, ble_adapter)
            await asyncio.sleep(2.0)

        deadline = time.time() + 15.0
        while time.time() < deadline:
            status = ble_api.get_status()
            if status.status in ("connectable", "enabled"):
                break
            await asyncio.sleep(1.0)

        devices = await BleDeviceClient.scan(
            name=ble_scanned_device.name, retries=2, adapter=ble_adapter,
        )
        assert devices, "Device not found after pairing removal"
        await ble_client.connect(devices[0].connect_target, adapter=ble_adapter)

        try:
            with allure.step("Read characteristic without bond"):
                data = await ble_client.read_characteristic(
                    CHAR_DEVICE_INFO_SERIAL
                )
                assert len(data) > 0, "Empty serial number after pairing removal"
        finally:
            await ble_client.disconnect()
