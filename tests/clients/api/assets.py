"""
Assets, Display, and Audio API client and Pydantic models.

Endpoints:
- POST /api/assets/upload
- DELETE /api/assets/upload
- POST /api/display/draw
- DELETE /api/display/draw
- POST /api/audio/play
- DELETE /api/audio/play
"""

from __future__ import annotations

import requests
from typing import Any

from pydantic import BaseModel, field_validator

from .base import BaseAPI

# Priority constants mirrored from loader.h
LOADER_MAX_PRIORITY = 100
LOADER_DEFAULT_APP_PRIORITY = 10  # any running app's baseline priority
LOADER_MAX_APP_PRIORITY = 90  # busy app sets this while a work session is active
LOADER_STUB_APP_PRIORITY = 0  # poweroff / settings stub apps; always preemptable
DEFAULT_ELEMENT_PRIORITY = 50  # default priority used by the draw endpoint when omitted

# Draw semantics: a POST /api/display/draw request is accepted when
#   request_priority >= active_loader_priority
# Equal-priority requests from a *different* app_id override the current display.


# === Response Models ===


class AssetResultResponse(BaseModel):
    """Generic asset/display/audio operation result."""

    result: str

    @field_validator("result")
    @classmethod
    def validate_result(cls, v: str) -> str:
        """Validate result indicates success."""
        assert v, "Expected non-empty result"
        return v


# === Request Models ===


class DisplayElement(BaseModel):
    """Single display element for draw command."""

    id: str
    type: str
    timeout: int | None = None
    text: str | None = None
    path: str | None = None
    x: int = 0
    y: int = 0
    align: str | None = None
    font: str | None = None
    color: str | None = None
    display: str | None = None


class DisplayDrawRequest(BaseModel):
    """Request for POST /api/display/draw."""

    app_id: str
    priority: int | None = None
    elements: list[DisplayElement]


# === API Client ===


class AssetsAPI(BaseAPI):
    """
    Assets, Display, and Audio API client.

    Assets endpoints:
    - POST /api/assets/upload - Upload asset
    - DELETE /api/assets/upload - Delete assets

    Display endpoints:
    - POST /api/display/draw - Draw to display
    - DELETE /api/display/draw - Clear display

    Audio endpoints:
    - POST /api/audio/play - Play audio
    - DELETE /api/audio/play - Stop audio
    """

    # === Assets ===

    def upload_asset(
        self, app_id: str, filename: str, content: bytes, timeout: int = 10
    ) -> AssetResultResponse:
        """
        Upload an asset file.

        Args:
            app_id: Application ID
            filename: Asset filename
            content: File content as bytes
            timeout: Request timeout in seconds
        """
        return self.post(
            "/api/assets/upload",
            AssetResultResponse,
            params={"app_id": app_id, "file": filename},
            data=content,
            headers={"Content-Type": "application/octet-stream"},
            timeout=timeout,
        )

    def delete_assets(self, app_id: str) -> AssetResultResponse:
        """
        Delete all assets for an application.

        Args:
            app_id: Application ID
        """
        return self.delete(
            "/api/assets/upload",
            AssetResultResponse,
            params={"app_id": app_id},
        )

    # === Display ===

    def draw(
        self,
        app_id: str,
        elements: list[dict[str, Any]],
        priority: int | None = None,
    ) -> AssetResultResponse:
        """
        Draw elements to the display (raises on non-2xx).

        Args:
            app_id: Application ID
            elements: List of element dictionaries
            priority: Draw priority (1–100). Defaults to server default (50).
        """
        body: dict[str, Any] = {"app_id": app_id, "elements": elements}
        if priority is not None:
            body["priority"] = priority
        return self.post(
            "/api/display/draw",
            AssetResultResponse,
            json=body,
        )

    def draw_response(
        self,
        app_id: str,
        elements: list[dict[str, Any]],
        priority: int | None = None,
    ) -> requests.Response:
        """
        Draw elements to the display and return the raw response.

        Use this variant when the call is expected to fail (400, 409, etc.)
        so that the error status code can be inspected without raising.
        """
        body: dict[str, Any] = {"app_id": app_id, "elements": elements}
        if priority is not None:
            body["priority"] = priority
        return self.post_raw("/api/display/draw", json=body)

    def draw_raw(self, data: dict[str, Any]) -> requests.Response:
        """Send a fully custom draw payload (for schema / limit testing)."""
        return self.post_raw("/api/display/draw", json=data)

    def clear_display(self) -> AssetResultResponse:
        """Clear all elements from the display."""
        return self.delete("/api/display/draw", AssetResultResponse)

    def clear_display_by_app(self, app_id: str) -> requests.Response:
        """Clear display elements belonging to a specific app_id."""
        return self.delete_raw("/api/display/draw", params={"app_id": app_id})

    # === Audio ===

    def play_audio(self, app_id: str, path: str) -> AssetResultResponse:
        """
        Play an audio file.

        Args:
            app_id: Application ID
            path: Audio file path
        """
        return self.post(
            "/api/audio/play",
            AssetResultResponse,
            params={"app_id": app_id, "path": path},
        )

    def stop_audio(self) -> AssetResultResponse:
        """Stop audio playback."""
        return self.delete("/api/audio/play", AssetResultResponse)
