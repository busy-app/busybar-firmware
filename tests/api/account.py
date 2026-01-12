"""
Account API client and Pydantic models.

Endpoints:
- GET /api/account/info
- GET /api/account/status
- GET /api/account/profile
- POST /api/account/profile
- POST /api/account/link
- DELETE /api/account
"""

from __future__ import annotations

import os
from typing import Literal

from pydantic import BaseModel

from .base import BaseAPI


# === MQTT Profile Configuration ===
# Can be overridden via environment variables

class MQTTProfiles:
    """MQTT server profiles for different environments."""

    PROD = os.environ.get("MQTT_PROD_URL", "mqtts://mqtt.cloud.busy.app:8883")
    DEV = os.environ.get("MQTT_DEV_URL", "mqtts://mqtt.cloud.dev.busy.app:8883")
    STAGE = os.environ.get("MQTT_STAGE_URL", "mqtts://mqtt.cloud.stage.busy.app:8883")
    TEST = os.environ.get("MQTT_TEST_URL", "mqtts://mqtt.cloud.test.busy.app:8883")

    # Default profile for tests
    DEFAULT = os.environ.get("MQTT_DEFAULT_PROFILE", "dev")


# === Response Models ===


class AccountInfoResponse(BaseModel):
    """Response from GET /api/account/info."""

    linked: bool
    id: str | None = None
    email: str | None = None
    user_id: str | None = None


class AccountStatusResponse(BaseModel):
    """Response from GET /api/account/status."""

    status: Literal["error", "disconnected", "connected"]


class AccountProfileResponse(BaseModel):
    """Response from GET /api/account/profile."""

    profile: Literal["dev", "prod", "local", "custom"]
    custom_url: str | None = None


class AccountLinkResponse(BaseModel):
    """Response from POST /api/account/link."""

    code: str
    expires_at: int


class AccountResultResponse(BaseModel):
    """Generic account operation result."""

    result: str


# === API Client ===


class AccountAPI(BaseAPI):
    """
    Account API client.

    Endpoints:
    - GET /api/account/info - Get account info
    - GET /api/account/status - Get MQTT connection status
    - GET /api/account/profile - Get backend profile
    - POST /api/account/profile - Set backend profile
    - POST /api/account/link - Request linking PIN
    - DELETE /api/account - Unlink account
    """

    def get_info(self) -> AccountInfoResponse:
        """Get account information."""
        return self.get("/api/account/info", AccountInfoResponse)

    def get_status(self) -> AccountStatusResponse:
        """Get MQTT connection status."""
        return self.get("/api/account/status", AccountStatusResponse)

    def get_profile(self) -> AccountProfileResponse:
        """Get current backend profile."""
        return self.get("/api/account/profile", AccountProfileResponse)

    def set_profile(self, profile: str, custom_url: str = None) -> AccountResultResponse:
        """
        Set backend profile.

        Args:
            profile: Profile name (dev, prod, local, custom)
            custom_url: Custom MQTT URL (required if profile is "custom")
        """
        params = {"profile": profile}
        if custom_url:
            params["custom_url"] = custom_url
        return self.post("/api/account/profile", AccountResultResponse, params=params)

    def set_profile_raw(self, profile: str, custom_url: str = None):
        """Set profile and return raw response (for error testing)."""
        params = {"profile": profile}
        if custom_url:
            params["custom_url"] = custom_url
        return self.post_raw("/api/account/profile", params=params)

    def link(self) -> AccountLinkResponse:
        """Request account linking PIN code."""
        return self.post("/api/account/link", AccountLinkResponse)

    def unlink(self) -> AccountResultResponse:
        """Unlink account."""
        return self.delete("/api/account", AccountResultResponse)

    # === Profile Switching Helpers ===

    def set_profile_prod(self) -> AccountResultResponse:
        """Switch to production MQTT profile."""
        return self.set_profile("prod")

    def set_profile_dev(self) -> AccountResultResponse:
        """Switch to development MQTT profile."""
        return self.set_profile("dev")

    def set_profile_stage(self) -> AccountResultResponse:
        """Switch to staging MQTT profile (custom URL)."""
        return self.set_profile("custom", MQTTProfiles.STAGE)

    def set_profile_test(self) -> AccountResultResponse:
        """Switch to test MQTT profile (custom URL)."""
        return self.set_profile("custom", MQTTProfiles.TEST)

    def set_default_profile(self) -> AccountResultResponse:
        """
        Switch to the default test profile.

        Uses MQTT_DEFAULT_PROFILE env var (defaults to 'dev').
        """
        profile_map = {
            "prod": ("prod", None),
            "dev": ("dev", None),
            "stage": ("custom", MQTTProfiles.STAGE),
            "test": ("custom", MQTTProfiles.TEST),
        }
        profile, url = profile_map.get(MQTTProfiles.DEFAULT, ("custom", MQTTProfiles.DEFAULT))
        return self.set_profile(profile, url)
