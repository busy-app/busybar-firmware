"""
Settings API client and Pydantic models.

Endpoints:
- GET /api/name
- POST /api/name
- GET /api/access
- POST /api/access
- GET /api/display/brightness
- POST /api/display/brightness
- GET /api/audio/volume
- POST /api/audio/volume
"""

from __future__ import annotations

from typing import Literal

from pydantic import BaseModel, field_validator

from .base import BaseAPI


# === Response Models ===


class NameResponse(BaseModel):
    """Response from GET /api/name."""

    name: str


class AccessResponse(BaseModel):
    """Response from GET /api/access."""

    mode: Literal["disabled", "enabled", "key"]
    key_valid: bool


class BrightnessResponse(BaseModel):
    """Response from GET /api/display/brightness."""

    front: str | int
    back: str | int


class VolumeResponse(BaseModel):
    """Response from GET /api/audio/volume."""

    volume: int | float


class SettingsResultResponse(BaseModel):
    """Generic settings operation result."""

    result: str

    @field_validator("result")
    @classmethod
    def validate_result(cls, v: str) -> str:
        """Validate result indicates success."""
        assert v, "Expected non-empty result"
        return v


# === Request Models ===


class SetNameRequest(BaseModel):
    """Request for POST /api/name."""

    name: str


# === API Client ===


class SettingsAPI(BaseAPI):
    """
    Settings API client.

    Name endpoints:
    - GET /api/name - Get device name
    - POST /api/name - Set device name

    Access endpoints:
    - GET /api/access - Get access settings
    - POST /api/access - Set access settings

    Display brightness:
    - GET /api/display/brightness - Get brightness
    - POST /api/display/brightness - Set brightness

    Audio volume:
    - GET /api/audio/volume - Get volume
    - POST /api/audio/volume - Set volume
    """

    # === Name ===

    def get_name(self) -> NameResponse:
        """Get device name."""
        return self.get("/api/name", NameResponse)

    def set_name(self, name: str) -> SettingsResultResponse:
        """
        Set device name.

        Args:
            name: New device name
        """
        req = SetNameRequest(name=name)
        return self.post("/api/name", SettingsResultResponse, json=req.model_dump())

    def set_name_raw(self, name: str):
        """Set name and return raw response (for error testing)."""
        return self.post_raw("/api/name", json={"name": name})

    # === Access ===

    def get_access(self) -> AccessResponse:
        """Get access settings."""
        return self.get("/api/access", AccessResponse)

    def set_access(self, mode: str, key: str = None) -> SettingsResultResponse:
        """
        Set access settings.

        Args:
            mode: Access mode (disabled, enabled, key)
            key: Access key (required if mode is "key")
        """
        params = {"mode": mode}
        if key is not None:
            params["key"] = key
        return self.post("/api/access", SettingsResultResponse, params=params)

    # === Brightness ===

    def get_brightness(self) -> BrightnessResponse:
        """Get display brightness."""
        return self.get("/api/display/brightness", BrightnessResponse)

    def set_brightness(self, front: str = None, back: str = None) -> SettingsResultResponse:
        """
        Set display brightness.

        Args:
            front: Front display brightness (0-100 or "auto")
            back: Back display brightness (0-100 or "auto")
        """
        params = {}
        if front is not None:
            params["front"] = front
        if back is not None:
            params["back"] = back
        return self.post("/api/display/brightness", SettingsResultResponse, params=params)

    # === Volume ===

    def get_volume(self) -> VolumeResponse:
        """Get audio volume."""
        return self.get("/api/audio/volume", VolumeResponse)

    def set_volume(self, volume: int) -> SettingsResultResponse:
        """
        Set audio volume.

        Args:
            volume: Volume level (0-100)
        """
        return self.post("/api/audio/volume", SettingsResultResponse, params={"volume": volume})

    def set_volume_raw(self, volume: int):
        """Set volume and return raw response (for error testing)."""
        return self.post_raw("/api/audio/volume", params={"volume": volume})
