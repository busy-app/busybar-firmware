"""
BLE client library for BSB Test Automation.

This module provides a host-side BLE client using bleak for direct
Bluetooth Low Energy communication with the BSB device.

Usage:
    from clients.ble import BleDeviceClient
    from clients.ble.models import DeviceInfoData, BatteryLevelData
    from clients.ble.constants import SERVICE_DEVICE_INFO

    async def test_device_info(connected_ble_client: BleDeviceClient):
        info = await connected_ble_client.read_device_info()
        assert info.serial_number
"""

from __future__ import annotations

from .client import BleDeviceClient
from .constants import (
    CHAR_BATTERY_LEVEL,
    CHAR_BATTERY_STATUS,
    CHAR_DEVICE_EVENTS_FLAGS,
    CHAR_DEVICE_INFO_HW_REV,
    CHAR_DEVICE_INFO_SERIAL,
    CHAR_DEVICE_INFO_SW_REV,
    CHAR_HM10_RX,
    CHAR_HM10_TX,
    CHAR_NUS_CNT,
    CHAR_NUS_RX,
    CHAR_NUS_TX,
    DEFAULT_DEVICE_NAME,
    SERVICE_BATTERY,
    SERVICE_DEVICE_EVENTS,
    SERVICE_DEVICE_INFO,
    SERVICE_HM10_UART,
    SERVICE_NORDIC_UART,
)
from .models import (
    BatteryLevelData,
    BatteryStatusData,
    DeviceEventsFlags,
    DeviceInfoData,
    GattCharacteristicInfo,
    GattServiceInfo,
    ScannedDevice,
)

__all__ = [
    "BleDeviceClient",
    "ScannedDevice",
    "GattServiceInfo",
    "GattCharacteristicInfo",
    "DeviceInfoData",
    "BatteryLevelData",
    "BatteryStatusData",
    "DeviceEventsFlags",
    "DEFAULT_DEVICE_NAME",
    "SERVICE_DEVICE_INFO",
    "SERVICE_BATTERY",
    "SERVICE_NORDIC_UART",
    "SERVICE_HM10_UART",
    "SERVICE_DEVICE_EVENTS",
    "CHAR_DEVICE_INFO_SERIAL",
    "CHAR_DEVICE_INFO_HW_REV",
    "CHAR_DEVICE_INFO_SW_REV",
    "CHAR_BATTERY_LEVEL",
    "CHAR_BATTERY_STATUS",
    "CHAR_NUS_RX",
    "CHAR_NUS_TX",
    "CHAR_NUS_CNT",
    "CHAR_HM10_TX",
    "CHAR_HM10_RX",
    "CHAR_DEVICE_EVENTS_FLAGS",
]
