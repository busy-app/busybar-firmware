"""
Account API client and Pydantic models.

Endpoints:
- GET /api/account/info
- GET /api/account/status
- GET /api/account/backend
- PUT /api/account/backend
- POST /api/account/link
- DELETE /api/account
"""

from __future__ import annotations

import os
from typing import Literal

from pydantic import BaseModel, field_validator

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
    """Response from GET /api/account/profile.

    .. deprecated::
        The /api/account/profile endpoint was replaced by /api/account/backend
        in FW-881.  This model is retained for reference only.
    """

    profile: Literal["dev", "prod", "local", "custom"]
    custom_url: str | None = None


class AccountBackend(BaseModel):
    """MQTT backend configuration from GET/PUT /api/account/backend."""

    server_url: str
    client_cert_type: Literal["default", "custom", "none"]
    ignore_server_cert: bool


class AccountLinkResponse(BaseModel):
    """Response from POST /api/account/link."""

    code: str
    expires_at: int


class AccountResultResponse(BaseModel):
    """Generic account operation result."""

    result: str

    @field_validator("result")
    @classmethod
    def validate_result(cls, v: str) -> str:
        """Validate result indicates success."""
        assert v, "Expected non-empty result"
        return v


# === API Client ===


class AccountAPI(BaseAPI):
    """
    Account API client.

    Endpoints:
    - GET /api/account/info - Get account info
    - GET /api/account/status - Get MQTT connection status
    - GET /api/account/backend - Get MQTT backend configuration
    - PUT /api/account/backend - Set MQTT backend configuration
    - POST /api/account/link - Request linking PIN
    - DELETE /api/account - Unlink account
    """

    def get_info(self) -> AccountInfoResponse:
        """Get account information."""
        return self.get("/api/account/info", AccountInfoResponse)

    def get_status(self) -> AccountStatusResponse:
        """Get MQTT connection status."""
        return self.get("/api/account/status", AccountStatusResponse)

    def get_backend(self) -> AccountBackend:
        """Get MQTT backend configuration."""
        return self.get("/api/account/backend", AccountBackend)

    def set_backend(self, config: AccountBackend) -> AccountResultResponse:
        """Set MQTT backend configuration."""
        return self.put(
            "/api/account/backend", AccountResultResponse, json=config.model_dump()
        )

    def set_backend_raw(self, payload: dict):
        """Set backend config and return raw response (for error testing)."""
        return self.put_raw("/api/account/backend", json=payload)

    def link(self) -> AccountLinkResponse:
        """Request account linking PIN code."""
        return self.post("/api/account/link", AccountLinkResponse, data=b"")

    def unlink(self) -> AccountResultResponse:
        """Unlink account."""
        return self.delete("/api/account", AccountResultResponse)

    # === Backend Switching Helpers ===

    def set_backend_default(self) -> AccountResultResponse:
        """Switch to the default (built-in) MQTT backend."""
        return self.set_backend(
            AccountBackend(
                server_url="default",
                client_cert_type="default",
                ignore_server_cert=False,
            )
        )

    def set_backend_custom(self, server_url: str) -> AccountResultResponse:
        """Switch to a custom MQTT backend URL."""
        return self.set_backend(
            AccountBackend(
                server_url=server_url,
                client_cert_type="default",
                ignore_server_cert=False,
            )
        )
