"""
Busy Timer API client and Pydantic models.

Endpoints:
- GET /api/busy/snapshot
- PUT /api/busy/snapshot
- GET /api/busy/profiles/{slot}
- PUT /api/busy/profiles/{slot}
"""

from __future__ import annotations

from typing import Any, Literal

from pydantic import BaseModel

from .base import BaseAPI


# === Response Models ===


class BusyBarSettings(BaseModel):
    """BUSY bar display settings."""

    theme: str
    show_work_phase_only: bool
    trigger_smart_home: bool
    show_work_time: bool = True


class BusySnapshotData(BaseModel):
    """Polymorphic snapshot data (NOT_STARTED, INFINITE, SIMPLE, INTERVAL)."""

    type: str
    card_id: str | None = None
    is_paused: bool | None = None
    time_left_ms: int | None = None
    current_interval: int | None = None
    current_interval_time_total_ms: int | None = None
    current_interval_time_left_ms: int | None = None
    interval_settings: dict | None = None
    busy_bar_settings: BusyBarSettings | None = None


class BusySnapshotResponse(BaseModel):
    """Response from GET /api/busy/snapshot."""

    snapshot: dict[str, Any]
    busy_bar_settings: BusyBarSettings | None = None
    snapshot_timestamp_ms: int


class BusyTimerSettings(BaseModel):
    """Polymorphic timer settings."""

    type: str
    total_time_ms: int | None = None
    interval_work_ms: int | None = None
    interval_rest_ms: int | None = None
    interval_work_cycles_count: int | None = None
    is_autostart_enabled: bool | None = None


class BusyProfileResponse(BaseModel):
    """Response from GET /api/busy/profiles/{slot}."""

    sort_order: int
    title: str
    id: str
    timer_settings: dict[str, Any]
    busy_bar_settings: BusyBarSettings
    profile_timestamp_ms: int


class BusyResultResponse(BaseModel):
    """Generic busy operation result."""

    result: str


# === API Client ===


class BusyAPI(BaseAPI):
    """
    Busy Timer API client.

    Endpoints:
    - GET /api/busy/snapshot - Get BUSY timer snapshot
    - PUT /api/busy/snapshot - Set BUSY timer snapshot
    - GET /api/busy/profiles/{slot} - Get BUSY timer profile
    - PUT /api/busy/profiles/{slot} - Set BUSY timer profile
    """

    def get_snapshot(self) -> BusySnapshotResponse:
        """Get current BUSY timer snapshot."""
        return self.get("/api/busy/snapshot", BusySnapshotResponse)

    def set_snapshot(self, snapshot_data: dict) -> BusyResultResponse:
        """Set BUSY timer snapshot."""
        return self.put(
            "/api/busy/snapshot",
            BusyResultResponse,
            json=snapshot_data,
        )

    def set_snapshot_raw(self, snapshot_data: dict):
        """Set BUSY timer snapshot and return raw response."""
        url = f"{self.base_url}/api/busy/snapshot"
        import allure
        with allure.step("PUT /api/busy/snapshot (raw)"):
            self._attach_request("PUT", url, json=snapshot_data)
            response = self.session.put(url, json=snapshot_data, timeout=10)
            self._attach_response(response)
            return response

    def get_profile(self, slot: str) -> BusyProfileResponse:
        """
        Get BUSY timer profile.

        Args:
            slot: Profile slot ("busy" or "custom")
        """
        return self.get(f"/api/busy/profiles/{slot}", BusyProfileResponse)

    def get_profile_raw(self, slot: str):
        """Get profile and return raw response (for error testing)."""
        return self.get_raw(f"/api/busy/profiles/{slot}")

    def set_profile(self, slot: str, profile_data: dict) -> BusyResultResponse:
        """
        Set BUSY timer profile.

        Args:
            slot: Profile slot ("busy" or "custom")
            profile_data: Profile data
        """
        return self.put(
            f"/api/busy/profiles/{slot}",
            BusyResultResponse,
            json=profile_data,
        )

    def set_profile_raw(self, slot: str, profile_data: dict):
        """Set profile and return raw response."""
        url = f"{self.base_url}/api/busy/profiles/{slot}"
        import allure
        with allure.step(f"PUT /api/busy/profiles/{slot} (raw)"):
            self._attach_request("PUT", url, json=profile_data)
            response = self.session.put(url, json=profile_data, timeout=10)
            self._attach_response(response)
            return response
