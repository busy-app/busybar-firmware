"""
BLE scanning / advertising tests.

Verifies that the device is discoverable and advertising the expected
name, manufacturer data, and RSSI values.
"""

from __future__ import annotations

import asyncio
import time

import allure
import pytest

from clients.ble.client import BleDeviceClient
from clients.ble.constants import (
    CHAR_GAP_DEVICE_NAME,
    DEFAULT_DEVICE_NAME,
    MANUFACTURER_ID,
)
from clients.ble.models import ScannedDevice


@allure.feature("BLE")
@allure.story("Scanning")
@pytest.mark.ble
class TestBleScanning:
    """Device discovery and advertising validation."""

    @allure.title("Device is discoverable by name")
    async def test_device_found_by_name(
        self,
        ble_scanned_device: ScannedDevice,
        ble_device_name: str,
    ) -> None:
        """The device should appear in scan results with the expected name."""
        assert ble_scanned_device.name is not None
        assert ble_device_name.lower() in ble_scanned_device.name.lower(), (
            f"Expected name containing '{ble_device_name}', "
            f"got '{ble_scanned_device.name}'"
        )

    @allure.title("Manufacturer data present in advertising")
    async def test_manufacturer_data(
        self, ble_scanned_device: ScannedDevice
    ) -> None:
        """Advertising should include manufacturer-specific data with ID 0x0E29."""
        assert ble_scanned_device.manufacturer_data, (
            "No manufacturer data in advertising"
        )
        assert MANUFACTURER_ID in ble_scanned_device.manufacturer_data, (
            f"Manufacturer ID 0x{MANUFACTURER_ID:04X} not found in "
            f"advertising data keys: {list(ble_scanned_device.manufacturer_data.keys())}"
        )

    @allure.title("RSSI is within a reasonable range")
    async def test_rssi_range(
        self, ble_scanned_device: ScannedDevice
    ) -> None:
        """RSSI should be between -100 dBm and 0 dBm."""
        rssi = ble_scanned_device.rssi
        assert -100 <= rssi <= 0, f"RSSI {rssi} dBm is out of expected range"

    @allure.title("Device is not visible when BLE is disabled")
    async def test_not_visible_when_disabled(
        self,
        ble_enabled: None,
        ble_api,
        ble_device_name: str,
        ble_adapter: str | None,
    ) -> None:
        """After disabling BLE, this specific suite-isolated device should disappear."""
        with allure.step("Disable BLE"):
            ble_api.disable()
            await asyncio.sleep(1.0)

        try:
            with allure.step("Scan for this suite device (expect not found)"):
                devices = await BleDeviceClient.scan(
                    name=ble_device_name, retries=1, adapter=ble_adapter,
                )
                assert len(devices) == 0, (
                    f"Device still visible after BLE disable: {devices}"
                )
        finally:
            with allure.step("Re-enable BLE"):
                ble_api.enable()
                deadline = time.time() + 30.0
                while time.time() < deadline:
                    status = ble_api.get_status()
                    if status.status in ("connectable", "enabled"):
                        break
                    await asyncio.sleep(1.0)

    @allure.title("Custom device name is advertised and exposed after pairing")
    async def test_custom_name_advertised_and_read_after_pairing(
        self,
        ble_enabled: None,
        settings_api,
        ble_api,
        ble_client: BleDeviceClient,
        ble_adapter: str | None,
        clear_linux_host_pairing,
    ) -> None:
        """A custom LOH/name should not fall back to 'BUSY Bar' after pairing."""
        original_name = settings_api.get_name().name
        custom_name = "busy.local"

        try:
            with allure.step("Set custom device name"):
                settings_api.set_name(custom_name)
                await asyncio.sleep(1.0)

            with allure.step("Scan by custom advertising name"):
                devices = await BleDeviceClient.scan(
                    name=custom_name,
                    retries=3,
                    adapter=ble_adapter,
                )
                assert devices, f"Device not found advertising custom name {custom_name!r}"
                assert all(
                    device.name and custom_name.lower() in device.name.lower()
                    for device in devices
                ), f"Scan returned devices without custom name: {devices}"

            device = devices[0]
            clear_linux_host_pairing(device.address, ble_adapter)

            with allure.step("Pair/connect after custom name is set"):
                devices = await BleDeviceClient.scan(
                    name=custom_name,
                    address=device.address,
                    retries=2,
                    adapter=ble_adapter,
                )
                assert devices, "Device not found by custom name after host pairing reset"
                await ble_client.connect(devices[0].connect_target, adapter=ble_adapter)

            with allure.step("Read GAP Device Name after pairing"):
                gap_name = (
                    await ble_client.read_characteristic(CHAR_GAP_DEVICE_NAME)
                ).decode("utf-8", errors="replace").rstrip("\x00")
                assert gap_name == custom_name, (
                    f"GAP Device Name stayed {gap_name!r}; expected {custom_name!r}"
                )
                assert gap_name != DEFAULT_DEVICE_NAME
        finally:
            await ble_client.disconnect()
            with allure.step("Restore original device name"):
                settings_api.set_name(original_name)
                await asyncio.sleep(1.0)
                ble_api.enable()
