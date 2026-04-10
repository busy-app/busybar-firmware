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


# === Response Models ===


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

    result: str


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

    def erase_pairing(self):
        """Erase all smart home pairing info."""
        return self.delete_raw("/api/smart_home/pairing")

    def get_switch_state(self) -> SmartHomeSwitchState:
        """Get smart home switch state."""
        return self.get("/api/smart_home/switch", SmartHomeSwitchState)

    def set_switch_state(self, state: bool):
        """
        Set smart home switch state.

        Args:
            state: Switch state (true/false)
        """
        return self.post_raw(
            "/api/smart_home/switch",
            json={"state": state},
        )
