"""
Input API client and Pydantic models.

Endpoints:
- POST /api/input
"""

from __future__ import annotations

from typing import Literal

from pydantic import BaseModel

from .base import BaseAPI


# === Response Models ===


class InputErrorResponse(BaseModel):
    """Error response from POST /api/input."""

    error: str


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
    """

    VALID_KEYS = [
        "up", "down", "ok", "back", "start",
        "busy", "off", "custom", "apps", "settings"
    ]

    def send_key(self, key: str):
        """
        Send a key event.

        Args:
            key: Key name (up, down, ok, back, start, busy, off, custom, apps, settings)

        Returns:
            Raw response
        """
        return self.post_raw("/api/input", params={"key": key})
