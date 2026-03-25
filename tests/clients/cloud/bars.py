"""
Cloud bar management API client.

Endpoints (prefix /api/v0/bars):
- POST /link - Link a bar with OTP pin
- GET /list - List linked bars
- DELETE /{id} - Unlink a bar
"""

from __future__ import annotations

from pydantic import BaseModel, ConfigDict

from clients.api.base import BaseAPI


# === Response Models ===


class BarInfo(BaseModel):
    model_config = ConfigDict(extra="allow")

    id: str
    hardware_id: str | None = None
    label: str | None = None


class BarListSuccess(BaseModel):
    model_config = ConfigDict(extra="allow")

    bars: list[BarInfo] = []


class BarListResponse(BaseModel):
    success: BarListSuccess


# === API Client ===


class CloudBarAPI(BaseAPI):
    """
    Cloud bar management API client.

    Endpoints:
    - POST /api/v0/bars/link - Link a bar with OTP pin
    - GET /api/v0/bars/list - List linked bars
    - DELETE /api/v0/bars/{id} - Unlink a bar
    """

    def link_bar(self, pin: str):
        """Submit OTP pin to link a bar. Expects 204 No Content."""
        return self.post_raw("/api/v0/bars/link", json={"pin": pin})

    def list_bars(self) -> BarListSuccess:
        """List all bars linked to the authenticated account."""
        resp = self.get("/api/v0/bars/list", BarListResponse)
        return resp.success

    def unlink_bar(self, bar_id: str):
        """Unlink a bar by ID. Expects 204 No Content."""
        return self.delete_raw(f"/api/v0/bars/{bar_id}")
