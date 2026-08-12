"""
BLE UUID constants, timeouts, and size limits.

All UUIDs are derived from firmware source files under
``applications/services/ble/service/``.
"""

from __future__ import annotations

# ---------------------------------------------------------------------------
# Advertising defaults
# ---------------------------------------------------------------------------

DEFAULT_DEVICE_NAME: str = "BUSY Bar"
"""Default BLE advertising name (from ble_worker.c:28)."""

MANUFACTURER_ID: int = 0x0E29
"""Manufacturer-specific data company ID in advertising payload."""

APPEARANCE: int = 0x0880
"""BLE appearance value."""

SERVICE_CLASS: int = 0x308A
"""Service class identifier used in advertising."""

# ---------------------------------------------------------------------------
# GATT Service UUIDs
# ---------------------------------------------------------------------------

SERVICE_GAP: str = "00001800-0000-1000-8000-00805f9b34fb"
"""Generic Access Profile service (SIG 0x1800)."""

CHAR_GAP_DEVICE_NAME: str = "00002a00-0000-1000-8000-00805f9b34fb"
"""GAP Device Name characteristic (Read)."""

SERVICE_DEVICE_INFO: str = "0000180a-0000-1000-8000-00805f9b34fb"
"""Device Information Service (SIG 0x180A)."""

SERVICE_BATTERY: str = "0000180f-0000-1000-8000-00805f9b34fb"
"""Battery Service (SIG 0x180F)."""

SERVICE_NORDIC_UART: str = "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
"""Nordic UART Service (NUS)."""

SERVICE_HM10_UART: str = "0000ffe0-0000-1000-8000-00805f9b34fb"
"""HM-10 UART Service."""

SERVICE_DEVICE_EVENTS: str = "af569d00-716a-452d-be64-66e465766c29"
"""Device Events Service (custom)."""

ALL_SERVICE_UUIDS: list[str] = [
    SERVICE_DEVICE_INFO,
    SERVICE_BATTERY,
    SERVICE_NORDIC_UART,
    SERVICE_HM10_UART,
    # SERVICE_DEVICE_EVENTS is not implemented in firmware — the device
    # exposes exactly the services above (see ble_service_index.h).
]
"""All GATT service UUIDs expected on the device."""

# ---------------------------------------------------------------------------
# Device Information Service characteristics
# ---------------------------------------------------------------------------

CHAR_DEVICE_INFO_SERIAL: str = "00002a25-0000-1000-8000-00805f9b34fb"
"""Serial Number String (Read)."""

CHAR_DEVICE_INFO_HW_REV: str = "00002a27-0000-1000-8000-00805f9b34fb"
"""Hardware Revision String (Read)."""

CHAR_DEVICE_INFO_SW_REV: str = "00002a26-0000-1000-8000-00805f9b34fb"
"""Software Revision String (Read)."""

# ---------------------------------------------------------------------------
# Battery Service characteristics
# ---------------------------------------------------------------------------

CHAR_BATTERY_LEVEL: str = "00002a19-0000-1000-8000-00805f9b34fb"
"""Battery Level (Read + Notify)."""

CHAR_BATTERY_STATUS: str = "00002bed-0000-1000-8000-00805f9b34fb"
"""Battery Status (Read + Notify)."""

# ---------------------------------------------------------------------------
# Nordic UART Service characteristics
# ---------------------------------------------------------------------------

CHAR_NUS_RX: str = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
"""NUS RX characteristic (Write)."""

CHAR_NUS_TX: str = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
"""NUS TX characteristic (Read + Indicate)."""

CHAR_NUS_CNT: str = "6e400004-b5a3-f393-e0a9-e50e24dcca9e"
"""NUS Counter characteristic (Read + Write)."""

# ---------------------------------------------------------------------------
# HM-10 UART Service characteristics
# ---------------------------------------------------------------------------

CHAR_HM10_TX: str = "0000ffe1-0000-1000-8000-00805f9b34fb"
"""HM-10 TX characteristic (Read + Notify)."""

CHAR_HM10_RX: str = "0000ffe2-0000-1000-8000-00805f9b34fb"
"""HM-10 RX characteristic (Read + Write)."""

# ---------------------------------------------------------------------------
# Device Events Service characteristics
# ---------------------------------------------------------------------------

CHAR_DEVICE_EVENTS_FLAGS: str = "af569d01-716a-452d-be64-66e465766c29"
"""Device Events Flags characteristic (Read + Indicate)."""

# ---------------------------------------------------------------------------
# Data size limits
# ---------------------------------------------------------------------------

NUS_MAX_PAYLOAD_BYTES: int = 237
"""Maximum payload size for a single NUS write."""

HM10_MAX_PAYLOAD_BYTES: int = 100
"""Maximum payload size for a single HM-10 write."""

# ---------------------------------------------------------------------------
# Timeouts (seconds)
# ---------------------------------------------------------------------------

TIMEOUT_SCAN: float = 15.0
"""BLE scan timeout."""

TIMEOUT_CONNECT: float = 15.0
"""BLE connection timeout."""

TIMEOUT_OPERATION: float = 10.0
"""Generic BLE read/write operation timeout."""

TIMEOUT_NOTIFICATION_WAIT: float = 10.0
"""Maximum wait for a notification/indication to arrive."""

# ---------------------------------------------------------------------------
# Retry defaults
# ---------------------------------------------------------------------------

SCAN_RETRIES: int = 3
"""Number of scan retry attempts before giving up."""
