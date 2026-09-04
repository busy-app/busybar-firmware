"""
Input API client and Pydantic models.

Endpoints:
- POST /api/input
- GET /api/input/switch
"""

from __future__ import annotations

from typing import Literal

from pydantic import BaseModel

from .base import BaseAPI


# === Response Models ===


class InputErrorResponse(BaseModel):
    """Error response from POST /api/input."""

    error: str


class InputSwitchResponse(BaseModel):
    """Response from GET /api/input/switch."""

    position: str


# === Request Models ===


ValidKey = Literal[
    "up", "down", "ok", "back", "start",
    "busy", "off", "custom", "apps", "settings"
]


class InputKeyRequest(BaseModel):
    """Request for POST /api/input."""

    key: str  # Can be any string, but only valid keys will succeed


# === API Client ===


class InputAPI(BaseAPI):
    """
    Input API client.

    Endpoints:
    - POST /api/input - Send key event
    - GET /api/input/switch - Get switch position
    """

    VALID_KEYS = [
        "up", "down", "ok", "back", "start",
        "busy", "off", "custom", "apps", "settings"
    ]

    SWITCH_POSITIONS = ["busy", "custom", "off", "apps", "settings"]

    def send_key(self, key: str):
        """
        Send a key event.

        Args:
            key: Key name (up, down, ok, back, start, busy, off, custom, apps, settings)

        Returns:
            Raw response
        """
        return self.post_raw("/api/input", params={"key": key}, data=b"")

    def get_switch(self) -> InputSwitchResponse:
        """Get the current mode switch position."""
        return self.get("/api/input/switch", InputSwitchResponse)
