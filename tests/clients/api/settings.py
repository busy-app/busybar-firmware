"""
Settings API client and Pydantic models.

Endpoints:
- GET /api/name
- POST /api/name
- GET /api/access
- POST /api/access
- GET /api/access/tokens
- POST /api/access/tokens
- DELETE /api/access/tokens
- DELETE /api/access/tokens/{short_id}
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


class StoredAccessToken(BaseModel):
    """Access-token metadata returned by the list endpoint."""

    short_id: str
    display_id: str
    name: str
    created_at: str
    last_used_at: str


class MintedAccessToken(StoredAccessToken):
    """New token returned once by the mint endpoint."""

    token: str


class AccessTokensResponse(BaseModel):
    """Response from GET /api/access/tokens."""

    tokens: list[StoredAccessToken]


class BrightnessResponse(BaseModel):
    """Response from GET /api/display/brightness."""

    value: str


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


class CreateAccessTokenRequest(BaseModel):
    """Request for POST /api/access/tokens."""

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

    Access-token endpoints:
    - GET /api/access/tokens - List token metadata
    - POST /api/access/tokens - Mint a token
    - DELETE /api/access/tokens/{short_id} - Revoke one token
    - DELETE /api/access/tokens - Revoke all tokens

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
        return self.post("/api/access", SettingsResultResponse, params=params, data=b"")

    def set_access_raw(self, mode: str, key: str = None):
        """Set access settings and return raw response (for error testing)."""
        params = {"mode": mode}
        if key is not None:
            params["key"] = key
        return self.post_raw("/api/access", params=params, data=b"")

    # === Access tokens ===

    def list_access_tokens(self, **kwargs) -> AccessTokensResponse:
        """List access-token metadata without exposing full token values."""
        return self.get("/api/access/tokens", AccessTokensResponse, **kwargs)

    def list_access_tokens_raw(self, **kwargs):
        """List access tokens and return the raw response."""
        return self.get_raw("/api/access/tokens", **kwargs)

    def mint_access_token(self, name: str, **kwargs) -> MintedAccessToken:
        """Mint a new access token. Its full value is returned only once."""
        req = CreateAccessTokenRequest(name=name)
        return self.post(
            "/api/access/tokens",
            MintedAccessToken,
            json=req.model_dump(),
            **kwargs,
        )

    def mint_access_token_raw(self, **kwargs):
        """Mint an access token and return the raw response."""
        return self.post_raw("/api/access/tokens", **kwargs)

    def revoke_access_token(self, short_id: str, **kwargs) -> SettingsResultResponse:
        """Revoke an access token by its short ID."""
        return self.delete(
            f"/api/access/tokens/{short_id}",
            SettingsResultResponse,
            **kwargs,
        )

    def revoke_access_token_raw(self, short_id: str, **kwargs):
        """Revoke one access token and return the raw response."""
        return self.delete_raw(f"/api/access/tokens/{short_id}", **kwargs)

    def revoke_all_access_tokens(self, **kwargs) -> SettingsResultResponse:
        """Revoke all access tokens."""
        return self.delete(
            "/api/access/tokens",
            SettingsResultResponse,
            **kwargs,
        )

    def revoke_all_access_tokens_raw(self, **kwargs):
        """Revoke all access tokens and return the raw response."""
        return self.delete_raw("/api/access/tokens", **kwargs)

    # === Brightness ===

    def get_brightness(self) -> BrightnessResponse:
        """Get display brightness."""
        return self.get("/api/display/brightness", BrightnessResponse)

    def set_brightness(self, value: str) -> SettingsResultResponse:
        """
        Set display brightness.

        Args:
            value: Brightness value (0-100 or "auto")
        """
        return self.post(
            "/api/display/brightness",
            SettingsResultResponse,
            params={"value": value},
            data=b"",
        )

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
        return self.post(
            "/api/audio/volume",
            SettingsResultResponse,
            params={"volume": volume},
            data=b"",
        )

    def set_volume_raw(self, volume: int):
        """Set volume and return raw response (for error testing)."""
        return self.post_raw("/api/audio/volume", params={"volume": volume}, data=b"")
