"""
BLE device client -- async wrapper around bleak with Allure integration.

Usage::

    client = BleDeviceClient()
    devices = await BleDeviceClient.scan(name="BUSY Bar")
    await client.connect(devices[0].connect_target)
    info = await client.read_device_info()
    await client.disconnect()

Or as an async context manager::

    async with BleDeviceClient() as client:
        await client.connect(address)
        info = await client.read_device_info()
"""

from __future__ import annotations

import asyncio
import logging
import platform
import struct
import time
from typing import Any, Callable

import allure
from bleak import BleakClient, BleakScanner
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData
from bleak.exc import BleakDBusError

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
    SCAN_RETRIES,
    TIMEOUT_CONNECT,
    TIMEOUT_NOTIFICATION_WAIT,
    TIMEOUT_OPERATION,
    TIMEOUT_SCAN,
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

LINUX_FIRST_GATT_TIMEOUT: float = 30.0


class BleDeviceClient:
    """Async BLE client wrapping bleak with Allure step annotations."""

    def __init__(self) -> None:
        self.logger = logging.getLogger("BleDeviceClient")
        self._client: BleakClient | None = None
        self._address: str | None = None
        self._linux_first_gatt_pending = False

    # -- async context manager -----------------------------------------------

    async def __aenter__(self) -> BleDeviceClient:
        return self

    async def __aexit__(self, exc_type: Any, exc_val: Any, exc_tb: Any) -> None:
        await self.disconnect()

    # -- properties ----------------------------------------------------------

    @property
    def is_connected(self) -> bool:
        """Return True if the underlying bleak client is connected."""
        return self._client is not None and self._client.is_connected

    def _is_linux(self) -> bool:
        return platform.system() == "Linux"

    def _resolve_gatt_timeout(self, timeout: float | None) -> float:
        if timeout is not None:
            return timeout
        if self._is_linux() and self._linux_first_gatt_pending:
            self.logger.info(
                "Using Linux first-GATT timeout grace: %.1fs",
                LINUX_FIRST_GATT_TIMEOUT,
            )
            return LINUX_FIRST_GATT_TIMEOUT
        return TIMEOUT_OPERATION

    def _mark_gatt_ready(self) -> None:
        self._linux_first_gatt_pending = False

    async def _prepare_linux_first_gatt(self, timeout: float) -> float:
        if not (self._is_linux() and self._linux_first_gatt_pending):
            return timeout
        if self._client is None:
            raise RuntimeError("Not connected")

        self.logger.info(
            "Pairing Linux BLE session before first secured GATT operation"
        )
        started_at = time.monotonic()
        paired = await asyncio.wait_for(self._client.pair(), timeout=timeout)
        if not paired:
            raise RuntimeError("Linux BLE pairing failed before first GATT operation")
        elapsed = time.monotonic() - started_at
        return max(timeout - elapsed, 1.0)

    # -- scanning ------------------------------------------------------------

    @staticmethod
    async def scan(
        name: str | None = DEFAULT_DEVICE_NAME,
        address: str | None = None,
        timeout: float = TIMEOUT_SCAN,
        retries: int = SCAN_RETRIES,
        adapter: str | None = None,
    ) -> list[ScannedDevice]:
        """Discover BLE devices, optionally filtering by *name* or *address*.

        Retries up to *retries* times if no matching device is found.

        Returns:
            List of :class:`ScannedDevice` instances.
        """
        logger = logging.getLogger("BleDeviceClient")
        result: list[ScannedDevice] = []

        for attempt in range(1, retries + 1):
            with allure.step(f"BLE scan (attempt {attempt}/{retries}, timeout={timeout}s)"):
                logger.info(
                    "BLE scan attempt %d/%d (name=%r, address=%r, timeout=%.1fs)",
                    attempt, retries, name, address, timeout,
                )
                scan_kwargs = {"timeout": timeout, "return_adv": True}
                if adapter:
                    scan_kwargs["adapter"] = adapter
                try:
                    device_map = await BleakScanner.discover(**scan_kwargs)
                except BleakDBusError as exc:
                    # BlueZ leaves a discovery half-started when the device drops
                    # mid-scan (StopDiscovery -> org.bluez.Error.InProgress). Don't
                    # let one wedged scan error every following test — retry, and if
                    # it never clears the fixture treats an empty result as a skip.
                    if "InProgress" not in str(exc):
                        raise
                    logger.warning(
                        "BLE scan hit BlueZ InProgress on attempt %d/%d, retrying: %s",
                        attempt, retries, exc,
                    )
                    await asyncio.sleep(2.0)
                    continue
                logger.debug("Discovered %d devices", len(device_map))

                for dev, adv in device_map.values():
                    scanned = ScannedDevice(
                        name=adv.local_name or dev.name,
                        address=dev.address,
                        ble_device=dev,
                        rssi=adv.rssi,
                        manufacturer_data={
                            k: bytes(v) for k, v in adv.manufacturer_data.items()
                        },
                        service_uuids=[str(u) for u in adv.service_uuids],
                    )

                    # Apply filters
                    if name and (scanned.name is None or name.lower() not in scanned.name.lower()):
                        continue
                    if address and scanned.address.lower() != address.lower():
                        continue

                    result.append(scanned)

                if result:
                    logger.info("Found %d matching device(s)", len(result))
                    return result

                logger.warning(
                    "No matching device found on attempt %d/%d", attempt, retries
                )

        return result

    # -- connection ----------------------------------------------------------

    async def connect(
        self,
        address_or_device: str | BLEDevice,
        timeout: float = TIMEOUT_CONNECT,
        adapter: str | None = None,
    ) -> None:
        """Connect to a BLE device by address or BLEDevice object."""
        with allure.step(f"BLE connect to {address_or_device}"):
            self.logger.info("Connecting to %s (timeout=%.1fs)", address_or_device, timeout)
            kwargs: dict[str, Any] = {"timeout": timeout}
            if adapter:
                kwargs["adapter"] = adapter
            self._client = BleakClient(address_or_device, **kwargs)
            try:
                await self._client.connect()
            except Exception as exc:
                msg = str(exc)
                if "Peer removed pairing information" in msg:
                    raise RuntimeError(
                        "Stale pairing data detected. On macOS: System Settings "
                        "→ Bluetooth → device → Forget This Device. On Linux: "
                        "bluetoothctl remove <MAC>"
                    ) from exc
                raise
            if isinstance(address_or_device, str):
                self._address = address_or_device
            else:
                self._address = address_or_device.address
            self._linux_first_gatt_pending = self._is_linux()
            self.logger.info("Connected to %s", self._address)

    async def disconnect(self) -> None:
        """Disconnect from the BLE device."""
        if self._client is None:
            return
        with allure.step("BLE disconnect"):
            self.logger.info("Disconnecting from %s", self._address)
            try:
                if self._client.is_connected:
                    await self._client.disconnect()
            except Exception as exc:
                if isinstance(exc, EOFError):
                    self.logger.warning(
                        "Disconnect EOFError ignored (%s): known BlueZ/dbus-fast "
                        "cleanup issue after a successful BLE session",
                        type(exc).__name__,
                    )
                else:
                    self.logger.warning(
                        "Disconnect error ignored (%s): %r",
                        type(exc).__name__,
                        exc,
                    )
            finally:
                self._linux_first_gatt_pending = False
                self._client = None
                self._address = None

    # -- GATT service discovery ----------------------------------------------

    async def discover_services(self) -> dict[str, GattServiceInfo]:
        """Return a mapping of service UUID -> GattServiceInfo."""
        with allure.step("Discover GATT services"):
            self.logger.info("Discovering GATT services")
            if self._client is None:
                raise RuntimeError("Not connected")

            services = self._client.services
            result: dict[str, GattServiceInfo] = {}
            for svc in services:
                chars: list[GattCharacteristicInfo] = []
                for char in svc.characteristics:
                    chars.append(
                        GattCharacteristicInfo(
                            uuid=char.uuid,
                            properties=list(char.properties),
                            description=char.description,
                        )
                    )
                result[svc.uuid] = GattServiceInfo(
                    uuid=svc.uuid,
                    characteristics=chars,
                )
            self.logger.debug("Found %d services", len(result))
            return result

    # -- low-level characteristic operations ---------------------------------

    async def read_characteristic(
        self, uuid: str, timeout: float | None = None
    ) -> bytes:
        """Read a characteristic by UUID."""
        with allure.step(f"Read characteristic {uuid}"):
            self.logger.debug("Reading characteristic %s", uuid)
            if self._client is None:
                raise RuntimeError("Not connected")
            effective_timeout = self._resolve_gatt_timeout(timeout)
            effective_timeout = await self._prepare_linux_first_gatt(
                effective_timeout
            )
            data = await asyncio.wait_for(
                self._client.read_gatt_char(uuid),
                timeout=effective_timeout,
            )
            self._mark_gatt_ready()
            self.logger.debug("Read %d bytes from %s", len(data), uuid)
            return bytes(data)

    async def write_characteristic(
        self,
        uuid: str,
        data: bytes,
        response: bool = False,
        timeout: float | None = None,
    ) -> None:
        """Write data to a characteristic by UUID."""
        with allure.step(f"Write {len(data)} bytes to characteristic {uuid}"):
            self.logger.debug("Writing %d bytes to %s", len(data), uuid)
            if self._client is None:
                raise RuntimeError("Not connected")
            effective_timeout = self._resolve_gatt_timeout(timeout)
            effective_timeout = await self._prepare_linux_first_gatt(
                effective_timeout
            )
            await asyncio.wait_for(
                self._client.write_gatt_char(uuid, data, response=response),
                timeout=effective_timeout,
            )
            self._mark_gatt_ready()
            self.logger.debug("Write complete to %s", uuid)

    async def start_notify(
        self, uuid: str, callback: Callable[[int, bytearray], None]
    ) -> None:
        """Subscribe to notifications/indications on a characteristic."""
        with allure.step(f"Subscribe to {uuid}"):
            self.logger.debug("Subscribing to %s", uuid)
            if self._client is None:
                raise RuntimeError("Not connected")
            effective_timeout = self._resolve_gatt_timeout(None)
            effective_timeout = await self._prepare_linux_first_gatt(
                effective_timeout
            )
            await asyncio.wait_for(
                self._client.start_notify(uuid, callback),
                timeout=effective_timeout,
            )
            self._mark_gatt_ready()
            self.logger.debug("Subscribed to %s", uuid)

    async def stop_notify(self, uuid: str) -> None:
        """Unsubscribe from notifications/indications on a characteristic."""
        with allure.step(f"Unsubscribe from {uuid}"):
            self.logger.debug("Unsubscribing from %s", uuid)
            if self._client is None:
                raise RuntimeError("Not connected")
            if not self._client.is_connected:
                self.logger.info(
                    "Skipping unsubscribe from %s because BLE client is disconnected",
                    uuid,
                )
                return
            await self._client.stop_notify(uuid)
            self.logger.debug("Unsubscribed from %s", uuid)

    # -- high-level helpers --------------------------------------------------

    async def read_serial_number(self) -> str:
        """Read the Device Information serial-number characteristic."""
        with allure.step("Read Device Information serial number"):
            value = (await self.read_characteristic(CHAR_DEVICE_INFO_SERIAL)).decode(
                "utf-8", errors="replace"
            )
            self.logger.info("Device serial number: %s", value)
            return value

    async def read_hardware_revision(self) -> str:
        """Read the Device Information hardware-revision characteristic."""
        with allure.step("Read Device Information hardware revision"):
            value = (await self.read_characteristic(CHAR_DEVICE_INFO_HW_REV)).decode(
                "utf-8", errors="replace"
            )
            self.logger.info("Device hardware revision: %s", value)
            return value

    async def read_software_revision(self) -> str:
        """Read the Device Information software-revision characteristic."""
        with allure.step("Read Device Information software revision"):
            value = (await self.read_characteristic(CHAR_DEVICE_INFO_SW_REV)).decode(
                "utf-8", errors="replace"
            )
            self.logger.info("Device software revision: %s", value)
            return value

    async def read_device_info(self) -> DeviceInfoData:
        """Read all Device Information Service characteristics."""
        with allure.step("Read Device Information"):
            serial = await self.read_serial_number()
            hw_rev = await self.read_hardware_revision()
            sw_rev = await self.read_software_revision()
            info = DeviceInfoData.model_validate(
                {
                    "serial_number": serial,
                    "hardware_revision": hw_rev,
                    "software_revision": sw_rev,
                }
            )
            self.logger.info(
                "Device Info: serial=%s, hw=%s, sw=%s",
                info.serial_number,
                info.hardware_revision,
                info.software_revision,
            )
            return info

    async def read_battery_level(self) -> BatteryLevelData:
        """Read battery level (0-100%)."""
        with allure.step("Read Battery Level"):
            data = await self.read_characteristic(CHAR_BATTERY_LEVEL)
            level = data[0] if data else 0
            result = BatteryLevelData.model_validate({"level": level})
            self.logger.info("Battery level: %d%%", result.level)
            return result

    async def read_battery_status(self) -> BatteryStatusData:
        """Read raw battery status bytes."""
        with allure.step("Read Battery Status"):
            data = await self.read_characteristic(CHAR_BATTERY_STATUS)
            result = BatteryStatusData(raw=bytes(data))
            self.logger.info("Battery status: %d bytes", len(result.raw))
            return result

    async def read_event_flags(self) -> DeviceEventsFlags:
        """Read Device Events flags characteristic."""
        with allure.step("Read Device Events Flags"):
            data = await self.read_characteristic(CHAR_DEVICE_EVENTS_FLAGS)
            result = DeviceEventsFlags.from_bytes(data)
            self.logger.info("Event flags value: 0x%08X", result.flags_value)
            return result

    async def read_nus_counter(self) -> int:
        """Read the NUS counter characteristic (uint32 LE)."""
        with allure.step("Read NUS Counter"):
            data = await self.read_characteristic(CHAR_NUS_CNT)
            value = struct.unpack_from("<I", data)[0] if len(data) >= 4 else 0
            self.logger.info("NUS counter: %d", value)
            return value

    async def write_nus_rx(self, payload: bytes) -> None:
        """Write data to the NUS RX characteristic."""
        with allure.step(f"Write NUS RX ({len(payload)} bytes)"):
            await self.write_characteristic(CHAR_NUS_RX, payload)

    async def reset_nus_session(self) -> None:
        """Reset the NUS session counter to zero."""
        with allure.step("Reset NUS session counter"):
            await self.write_characteristic(
                CHAR_NUS_CNT, struct.pack("<I", 0), response=True
            )
            self.logger.info("NUS session counter reset to 0")
