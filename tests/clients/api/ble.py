"""
BLE API client and Pydantic models.

Endpoints:
- GET /api/ble/status
- POST /api/ble/enable
- POST /api/ble/disable
- DELETE /api/ble/pairing
"""

from __future__ import annotations

from typing import Literal

import requests
from pydantic import BaseModel

from .base import BaseAPI


# === Response Models ===


class BleStatusResponse(BaseModel):
    """Response from GET /api/ble/status."""

    status: Literal[
        "reset", "initialization", "disabled",
        "enabled", "connectable", "connected", "internal error"
    ]
    address: str | None = None


# === API Client ===


class BleAPI(BaseAPI):
    """
    BLE API client.

    Endpoints:
    - GET /api/ble/status - Get BLE status
    - POST /api/ble/enable - Enable BLE
    - POST /api/ble/disable - Disable BLE
    - DELETE /api/ble/pairing - Remove BLE pairing
    """

    def get_status(self) -> BleStatusResponse:
        """Get BLE status."""
        return self.get("/api/ble/status", BleStatusResponse)

    def enable(self) -> requests.Response:
        """Enable BLE."""
        return self.post_raw("/api/ble/enable", data=b"")

    def disable(self) -> requests.Response:
        """Disable BLE."""
        return self.post_raw("/api/ble/disable", data=b"")

    def remove_pairing(self) -> requests.Response:
        """Remove BLE pairing. May return 200 or 503."""
        return self.delete_raw("/api/ble/pairing")
