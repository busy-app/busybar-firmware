"""
GATT service discovery tests.

Verifies that all expected services and their expected characteristics are
present.
"""

from __future__ import annotations

import allure
import pytest

from clients.ble.client import BleDeviceClient
from clients.ble.constants import (
    ALL_SERVICE_UUIDS,
    CHAR_BATTERY_LEVEL,
    CHAR_BATTERY_STATUS,
    CHAR_DEVICE_INFO_HW_REV,
    CHAR_DEVICE_INFO_SERIAL,
    CHAR_DEVICE_INFO_SW_REV,
    CHAR_HM10_RX,
    CHAR_HM10_TX,
    CHAR_NUS_CNT,
    CHAR_NUS_RX,
    CHAR_NUS_TX,
    SERVICE_BATTERY,
    SERVICE_DEVICE_INFO,
    SERVICE_HM10_UART,
    SERVICE_NORDIC_UART,
)
from clients.ble.models import GattServiceInfo


@allure.feature("BLE")
@allure.story("Service Discovery")
@pytest.mark.ble
class TestBleServiceDiscovery:
    """GATT service and characteristic presence validation."""

    @allure.title("All expected GATT services are present")
    async def test_all_services_present(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        """Every expected service UUID should appear in the service list."""
        services = await connected_ble_client.discover_services()
        discovered_uuids = set(services.keys())
        for expected in ALL_SERVICE_UUIDS:
            assert expected in discovered_uuids, (
                f"Service {expected} not found. Discovered: {discovered_uuids}"
            )

    @allure.title("Device Info service has correct characteristics")
    async def test_device_info_characteristics(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        services = await connected_ble_client.discover_services()
        svc = services.get(SERVICE_DEVICE_INFO)
        assert svc is not None, "Device Info service not found"

        char_uuids = {c.uuid for c in svc.characteristics}
        for expected in [
            CHAR_DEVICE_INFO_SERIAL,
            CHAR_DEVICE_INFO_HW_REV,
            CHAR_DEVICE_INFO_SW_REV,
        ]:
            assert expected in char_uuids, (
                f"Characteristic {expected} not found in Device Info service"
            )

    @allure.title("Battery service has correct characteristics")
    async def test_battery_characteristics(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        services = await connected_ble_client.discover_services()
        svc = services.get(SERVICE_BATTERY)
        assert svc is not None, "Battery service not found"

        char_uuids = {c.uuid for c in svc.characteristics}
        for expected in [CHAR_BATTERY_LEVEL, CHAR_BATTERY_STATUS]:
            assert expected in char_uuids, (
                f"Characteristic {expected} not found in Battery service"
            )

    @allure.title("Nordic UART service has correct characteristics")
    async def test_nus_characteristics(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        services = await connected_ble_client.discover_services()
        svc = services.get(SERVICE_NORDIC_UART)
        assert svc is not None, "Nordic UART service not found"

        char_uuids = {c.uuid for c in svc.characteristics}
        for expected in [CHAR_NUS_RX, CHAR_NUS_TX, CHAR_NUS_CNT]:
            assert expected in char_uuids, (
                f"Characteristic {expected} not found in NUS service"
            )

    @allure.title("HM-10 UART service has correct characteristics")
    async def test_hm10_characteristics(
        self, connected_ble_client: BleDeviceClient
    ) -> None:
        services = await connected_ble_client.discover_services()
        svc = services.get(SERVICE_HM10_UART)
        assert svc is not None, "HM-10 UART service not found"

        char_uuids = {c.uuid for c in svc.characteristics}
        for expected in [CHAR_HM10_TX, CHAR_HM10_RX]:
            assert expected in char_uuids, (
                f"Characteristic {expected} not found in HM-10 service"
            )
