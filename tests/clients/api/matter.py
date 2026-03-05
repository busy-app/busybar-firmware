"""
Matter API client and Pydantic models.

Endpoints:
- GET /api/matter/commissioning
- POST /api/matter/commissioning
- DELETE /api/matter/commissioning
- GET /api/matter/endpoint/1
- POST /api/matter/endpoint/1
"""

from __future__ import annotations

from typing import Literal

from pydantic import BaseModel

from .base import BaseAPI


# === Response Models ===


class CommissioningStatus(BaseModel):
    """Commissioning status details."""

    value: Literal["never_started", "started", "completed_successfully", "failed"]
    timestamp: str | None = None


class MatterCommissioningResponse(BaseModel):
    """Response from GET /api/matter/commissioning."""

    fabric_count: int
    latest_commissioning_status: CommissioningStatus


class MatterCommissioningPayload(BaseModel):
    """Response from POST /api/matter/commissioning."""

    available_until: str
    qr_code: str
    manual_code: str


class MatterEndpointState(BaseModel):
    """Response from GET /api/matter/endpoint/1."""

    type: Literal["switch"]
    state: bool
    startup: str | None = None


class MatterResultResponse(BaseModel):
    """Generic matter operation result."""

    result: str


# === API Client ===


class MatterAPI(BaseAPI):
    """
    Matter API client.

    Endpoints:
    - GET /api/matter/commissioning - Get commissioning status
    - POST /api/matter/commissioning - Start commissioning
    - DELETE /api/matter/commissioning - Erase all commissioning
    - GET /api/matter/endpoint/1 - Get endpoint 1 state
    - POST /api/matter/endpoint/1 - Set endpoint 1 state
    """

    def get_commissioning(self) -> MatterCommissioningResponse:
        """Get Matter commissioning status."""
        return self.get("/api/matter/commissioning", MatterCommissioningResponse)

    def start_commissioning(self):
        """Start Matter commissioning. Returns payload or 503 error."""
        return self.post_raw("/api/matter/commissioning", data=b"")

    def erase_commissioning(self):
        """Erase all Matter commissioning info."""
        return self.delete_raw("/api/matter/commissioning")

    def get_endpoint_state(self) -> MatterEndpointState:
        """Get Matter endpoint 1 state."""
        return self.get("/api/matter/endpoint/1", MatterEndpointState)

    def set_endpoint_state(self, state: bool):
        """
        Set Matter endpoint 1 state.

        Args:
            state: Switch state (true/false)
        """
        return self.post_raw(
            "/api/matter/endpoint/1",
            json={"type": "switch", "state": state},
        )
