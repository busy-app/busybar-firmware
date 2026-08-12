"""
Device Information Service tests.

Reads serial number, hardware revision, and software revision over BLE
and cross-checks with the CLI device_info command.
"""

from __future__ import annotations

import allure
import pytest

from clients.ble.client import BleDeviceClient


@allure.feature("BLE")
@allure.story("Device Info")
@pytest.mark.ble
class TestBleDeviceInfo:
    """Device Information Service characteristic reads."""

    @allure.title("Read serial number")
    async def test_read_serial_number(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """Serial number should be a non-empty string."""
        serial = await connected_ble_client.read_serial_number()
        assert serial, "Serial number is empty"

    @allure.title("Read hardware revision")
    async def test_read_hardware_revision(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """Hardware revision should be a non-empty string."""
        hardware_revision = await connected_ble_client.read_hardware_revision()
        assert hardware_revision, "Hardware revision is empty"

    @allure.title("Read software revision")
    async def test_read_software_revision(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """Software revision should be a non-empty string."""
        software_revision = await connected_ble_client.read_software_revision()
        assert software_revision, "Software revision is empty"

    @allure.title("Cross-check serial number with CLI device_info")
    async def test_serial_matches_cli_device_info(
        self,
        connected_ble_client: BleDeviceClient,
        cli_device_info: str,
    ) -> None:
        """Serial number read over BLE should match the U5 hardware UID from CLI."""
        ble_serial = await connected_ble_client.read_serial_number()

        cli_serial = None
        for line in cli_device_info.splitlines():
            if line.strip().startswith("u5_hardware_uid"):
                _, value = line.split(":", 1)
                cli_serial = value.strip()
                break

        if cli_serial is None:
            pytest.fail("CLI device_info does not expose u5_hardware_uid")

        assert ble_serial == cli_serial, (
            f"BLE serial '{ble_serial}' != CLI u5_hardware_uid '{cli_serial}'"
        )
