"""
Smart Home API client and Pydantic models.

Endpoints:
- GET /api/smart_home/pairing
- POST /api/smart_home/pairing
- DELETE /api/smart_home/pairing
- GET /api/smart_home/switch
- POST /api/smart_home/switch
"""

from __future__ import annotations

from typing import Literal

from pydantic import BaseModel

from .base import BaseAPI


class PairingStatus(BaseModel):
    """Pairing status details."""

    value: Literal["never_started", "started", "completed_successfully", "failed"]
    timestamp: int | None = None


class SmartHomePairingResponse(BaseModel):
    """Response from GET /api/smart_home/pairing."""

    fabric_count: int
    latest_pairing_status: PairingStatus


class SmartHomePairingPayload(BaseModel):
    """Response from POST /api/smart_home/pairing."""

    available_until: str
    qr_code: str
    manual_code: str


class SmartHomeSwitchState(BaseModel):
    """Response from GET /api/smart_home/switch."""

    state: bool
    startup: str | None = None


class SmartHomeResultResponse(BaseModel):
    """Generic smart home operation result."""

    result: Literal["OK"]


class SmartHomeErrorResponse(BaseModel):
    """Generic smart home error response."""

    error: str


# === API Client ===


class SmartHomeAPI(BaseAPI):
    """
    Smart Home API client.

    Endpoints:
    - GET /api/smart_home/pairing - Get pairing status
    - POST /api/smart_home/pairing - Start pairing
    - DELETE /api/smart_home/pairing - Erase all pairing info
    - GET /api/smart_home/switch - Get switch state
    - POST /api/smart_home/switch - Set switch state
    """

    def get_pairing(self) -> SmartHomePairingResponse:
        """Get smart home pairing status."""
        return self.get("/api/smart_home/pairing", SmartHomePairingResponse)

    def start_pairing(self):
        """Start smart home pairing. Returns payload or 503 error."""
        return self.post_raw("/api/smart_home/pairing", data=b"")

    def erase_pairing(self) -> SmartHomeResultResponse:
        """Erase all smart home pairing info."""
        return self.delete(
            "/api/smart_home/pairing", SmartHomeResultResponse
        )

    def get_switch_state(self) -> SmartHomeSwitchState:
        """Get smart home switch state."""
        return self.get("/api/smart_home/switch", SmartHomeSwitchState)

    def set_switch_state(
        self, state: bool, startup: str = None
    ):
        """
        Set smart home switch state and/or startup mode.

        Args:
            state: Switch state (true/false)
            startup: Optional startup mode: "off", "on", "toggle", "last"
        """
        body = {"state": state}
        if startup is not None:
            body["startup"] = startup
        return self.post_raw("/api/smart_home/switch", json=body)

    def set_switch_startup(self, startup: str):
        """
        Set smart home switch startup mode only.

        Args:
            startup: Startup mode: "off", "on", "toggle", "last"
        """
        return self.post_raw("/api/smart_home/switch", json={"startup": startup})
