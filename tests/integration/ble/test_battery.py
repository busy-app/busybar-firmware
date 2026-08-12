"""
Battery Service tests.

Reads battery level and status over BLE, validates ranges, and tests
notification delivery.
"""

import asyncio

import allure
import pytest
from bleak.exc import BleakDBusError

from clients.ble.client import BleDeviceClient
from clients.ble.constants import CHAR_BATTERY_LEVEL, TIMEOUT_NOTIFICATION_WAIT
from clients.ble.models import BatteryLevelData, BatteryStatusData


@allure.feature("BLE")
@allure.story("Battery")
@pytest.mark.ble
class TestBleBattery:
    """Battery Service characteristic reads and notifications."""

    @allure.title("Battery level is 0-100 %")
    async def test_battery_level_range(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """Battery level should be an integer in the 0-100 range."""
        result = await connected_ble_client.read_battery_level()
        assert isinstance(result, BatteryLevelData)
        assert 0 <= result.level <= 100

    @allure.title("Battery status returns 3 bytes")
    async def test_battery_status_length(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """Battery status characteristic should return 3 bytes."""
        result = await connected_ble_client.read_battery_status()
        assert isinstance(result, BatteryStatusData)
        assert len(result.raw) == 3, (
            f"Expected 3 bytes, got {len(result.raw)}"
        )

    @allure.title("Battery level notification")
    @pytest.mark.skip(reason="Device-side notifications never arrive on the CI bench (see also NUS TX indications)")
    async def test_battery_level_notification(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """Subscribing to battery level should deliver at least one notification."""
        received: asyncio.Future[bytearray] = asyncio.get_event_loop().create_future()

        def _callback(sender: int, data: bytearray) -> None:
            if not received.done():
                received.set_result(data)

        await connected_ble_client.start_notify(CHAR_BATTERY_LEVEL, _callback)
        try:
            data = await asyncio.wait_for(
                received, timeout=TIMEOUT_NOTIFICATION_WAIT
            )
            level = data[0] if data else -1
            assert 0 <= level <= 100, f"Notification level {level} out of range"
        except asyncio.TimeoutError:
            pytest.fail(
                "No battery level notification received within timeout"
            )
        finally:
            await connected_ble_client.stop_notify(CHAR_BATTERY_LEVEL)

    @allure.title("Battery level consistent across reads")
    async def test_battery_level_consistency(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """Two consecutive reads should return the same (or very close) value."""
        first = await connected_ble_client.read_battery_level()
        try:
            second = await connected_ble_client.read_battery_level()
        except BleakDBusError as exc:
            if "ATT error: 0x0e" in str(exc):
                pytest.fail(
                    "Device/BlueZ path cannot perform a second secured GATT read "
                    "in the same BLE session (ATT 0x0e)"
                )
            raise
        assert abs(first.level - second.level) <= 1, (
            f"Battery level changed by {abs(first.level - second.level)}% "
            f"between consecutive reads ({first.level}% -> {second.level}%)"
        )
