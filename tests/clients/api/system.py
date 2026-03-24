"""
System API client and Pydantic models.

Endpoints:
- GET /api/version
- GET /api/status
- GET /api/status/device
- GET /api/status/firmware
- GET /api/status/system
- GET /api/status/power
- GET /api/time
- POST /api/time/timestamp
- GET /api/time/timezone
- POST /api/time/timezone
- GET /api/time/tzlist
"""

from __future__ import annotations

from typing import Literal

import requests
from pydantic import BaseModel, Field, field_validator

from .base import BaseAPI


# === Response Models ===


class VersionResponse(BaseModel):
    """Response from GET /api/version."""

    api_semver: str

    @field_validator("api_semver")
    @classmethod
    def validate_semver(cls, v: str) -> str:
        """Validate semantic version format (x.y.z)."""
        parts = v.split(".")
        if len(parts) != 3:
            raise ValueError(f"Invalid semver format: {v}, expected x.y.z")
        try:
            major, minor, patch = int(parts[0]), int(parts[1]), int(parts[2])
            if major < 0 or minor < 0 or patch < 0:
                raise ValueError(f"Version components must be non-negative: {v}")
        except ValueError as e:
            raise ValueError(f"Invalid semver format: {v}") from e
        return v


class DeviceInfo(BaseModel):
    """Device information from /api/status/device."""

    serial_number: str
    usb_mac: str
    wifi_mac: str | None = None
    ble_mac: str | None = None
    otp_valid: bool
    otp_model: str | None = None
    otp_timestamp: int | None = None


class FirmwareInfo(BaseModel):
    """Firmware information from /api/status/firmware."""

    version: str
    target: int
    branch: str
    build_date: str
    commit_hash: str
    nwp_version: str | None = None


class SystemInfo(BaseModel):
    """System information from /api/status or /api/status/system."""

    api_semver: str
    uptime: str
    boot_time: int


class PowerInfo(BaseModel):
    """Power information from /api/status or /api/status/power."""

    state: Literal["discharging", "charging", "charged"]
    battery_charge: int = Field(ge=0, le=100)
    battery_voltage: int
    battery_current: int
    usb_voltage: int


class StatusResponse(BaseModel):
    """Response from GET /api/status."""

    device: DeviceInfo
    firmware: FirmwareInfo
    system: SystemInfo
    power: PowerInfo


class TimeResponse(BaseModel):
    """Response from GET /api/time."""

    timestamp: str

    @field_validator("timestamp")
    @classmethod
    def validate_timestamp(cls, v: str) -> str:
        """Validate ISO 8601 format (basic check)."""
        if "T" not in v:
            raise ValueError(f"Timestamp should be in ISO 8601 format: {v}")
        return v


class TimezoneResponse(BaseModel):
    """Response from GET /api/time/timezone."""

    name: str
    offset: str
    abbr: str | None = None


class TimezoneItem(BaseModel):
    """Single timezone in timezone list."""

    name: str
    offset: str
    abbr: str | None = None


class TimezoneListResponse(BaseModel):
    """Response from GET /api/time/tzlist."""

    list: list[TimezoneItem]


class ResultResponse(BaseModel):
    """Generic response with result field."""

    result: str

    @field_validator("result")
    @classmethod
    def validate_result(cls, v: str) -> str:
        """Validate result indicates success."""
        assert v, "Expected non-empty result"
        return v


# === Request Models ===


class SetTimestampRequest(BaseModel):
    """Request for POST /api/time/timestamp."""

    timestamp: str  # ISO 8601 format, e.g., "2025-06-15T12:30:45"


class SetTimezoneRequest(BaseModel):
    """Request for POST /api/time/timezone."""

    timezone: str  # Timezone name (e.g. "Berlin", "London")

    @field_validator("timezone")
    @classmethod
    def validate_timezone(cls, v: str) -> str:
        """Validate timezone name is not empty."""
        if not v or not v.strip():
            raise ValueError("Timezone name cannot be empty")
        return v


# === API Client ===


class SystemAPI(BaseAPI):
    """
    System API client.

    Endpoints:
    - GET /api/version - Get API version
    - GET /api/status - Get full system status
    - GET /api/status/device - Get device info
    - GET /api/status/firmware - Get firmware info
    - GET /api/status/system - Get system info only
    - GET /api/status/power - Get power info only
    - GET /api/time - Get current time
    - POST /api/time/timestamp - Set timestamp
    - GET /api/time/timezone - Get timezone
    - POST /api/time/timezone - Set timezone
    - GET /api/time/tzlist - Get supported timezones
    """

    def get_version(self) -> VersionResponse:
        """Get API version."""
        return self.get("/api/version", VersionResponse)

    def get_status(self) -> StatusResponse:
        """Get full system status (system + power)."""
        return self.get("/api/status", StatusResponse)

    def get_device_info(self) -> DeviceInfo:
        """Get device information."""
        return self.get("/api/status/device", DeviceInfo)

    def get_firmware_info(self) -> FirmwareInfo:
        """Get firmware information."""
        return self.get("/api/status/firmware", FirmwareInfo)

    def get_system_status(self) -> SystemInfo:
        """Get system information only."""
        return self.get("/api/status/system", SystemInfo)

    def get_power_status(self) -> PowerInfo:
        """Get power information only."""
        return self.get("/api/status/power", PowerInfo)

    def get_time(self) -> TimeResponse:
        """Get current device time."""
        return self.get("/api/time", TimeResponse)

    def set_timestamp(self, timestamp: str) -> ResultResponse:
        """
        Set device timestamp.

        Args:
            timestamp: ISO 8601 format, e.g., "2025-06-15T12:30:45"

        Returns:
            ResultResponse with operation result
        """
        req = SetTimestampRequest(timestamp=timestamp)
        return self.post(
            "/api/time/timestamp",
            ResultResponse,
            params=req.model_dump(),
            data=b"",
        )

    def set_timestamp_raw(self, timestamp: str) -> requests.Response:
        """Set timestamp and return raw response (for error testing)."""
        return self.post_raw("/api/time/timestamp", params={"timestamp": timestamp}, data=b"")

    def set_timezone(self, timezone: str) -> ResultResponse:
        """
        Set device timezone.

        Args:
            timezone: Offset format, e.g., "+00:00", "-05:00"

        Returns:
            ResultResponse with operation result
        """
        req = SetTimezoneRequest(timezone=timezone)
        return self.post(
            "/api/time/timezone",
            ResultResponse,
            params=req.model_dump(),
            data=b"",
        )

    def get_timezone(self) -> TimezoneResponse:
        """Get current timezone."""
        return self.get("/api/time/timezone", TimezoneResponse)

    def set_timezone_raw(self, timezone: str) -> requests.Response:
        """Set timezone and return raw response (for error testing)."""
        return self.post_raw("/api/time/timezone", params={"timezone": timezone}, data=b"")

    def get_timezone_list(self) -> TimezoneListResponse:
        """Get list of supported timezones."""
        return self.get("/api/time/tzlist", TimezoneListResponse)
