"""
Pydantic v2 models for BLE data read from GATT characteristics.
"""

from __future__ import annotations

from typing import Any

from bleak.backends.device import BLEDevice
from pydantic import BaseModel, Field


# ---------------------------------------------------------------------------
# Scanning
# ---------------------------------------------------------------------------


class ScannedDevice(BaseModel):
    """Represents a BLE device discovered during scanning."""

    name: str | None = None
    address: str
    ble_device: BLEDevice | None = None
    rssi: int = 0
    manufacturer_data: dict[int, bytes] = Field(default_factory=dict)
    service_uuids: list[str] = Field(default_factory=list)

    model_config = {"arbitrary_types_allowed": True}

    @property
    def connect_target(self) -> str | BLEDevice:
        """Prefer the native Bleak device to avoid rediscovery on connect."""
        return self.ble_device or self.address


# ---------------------------------------------------------------------------
# GATT service / characteristic descriptors
# ---------------------------------------------------------------------------


class GattCharacteristicInfo(BaseModel):
    """Descriptor for a single GATT characteristic."""

    uuid: str
    properties: list[str] = Field(default_factory=list)
    description: str | None = None


class GattServiceInfo(BaseModel):
    """Descriptor for a GATT service and its characteristics."""

    uuid: str
    characteristics: list[GattCharacteristicInfo] = Field(default_factory=list)


# ---------------------------------------------------------------------------
# Device Information Service
# ---------------------------------------------------------------------------


class DeviceInfoData(BaseModel):
    """Data read from the Device Information Service."""

    serial_number: str
    hardware_revision: str
    software_revision: str


# ---------------------------------------------------------------------------
# Battery Service
# ---------------------------------------------------------------------------


class BatteryLevelData(BaseModel):
    """Battery level percentage."""

    level: int = Field(ge=0, le=100)


class BatteryStatusData(BaseModel):
    """Raw battery status bytes."""

    raw: bytes

    model_config = {"arbitrary_types_allowed": True}


# ---------------------------------------------------------------------------
# Device Events
# ---------------------------------------------------------------------------


class DeviceEventsFlags(BaseModel):
    """Device Events flags bitmask."""

    raw: bytes
    flags_value: int = 0

    model_config = {"arbitrary_types_allowed": True}

    @classmethod
    def from_bytes(cls, data: bytes) -> DeviceEventsFlags:
        """Create from raw characteristic bytes (little-endian uint32)."""
        value = int.from_bytes(data[:4], byteorder="little") if len(data) >= 4 else 0
        return cls(raw=data, flags_value=value)
