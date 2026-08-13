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

from typing import Any

import requests
from pydantic import BaseModel, field_validator

from .base import BaseAPI

# Priority constants mirrored from loader.h
LOADER_MAX_PRIORITY = 100
LOADER_DEFAULT_APP_PRIORITY = 10  # any running app's baseline priority
LOADER_MAX_APP_PRIORITY = 90  # high app priority below the HTTP draw ceiling
LOADER_STUB_APP_PRIORITY = 0  # poweroff / settings stub apps; always preemptable
LOADER_PASSTHROUGH_PRIORITY = LOADER_DEFAULT_APP_PRIORITY - 1
LOADER_BLOCKING_PRIORITY = LOADER_MAX_PRIORITY + 1
DEFAULT_ELEMENT_PRIORITY = 50  # default priority used by the draw endpoint when omitted

# Draw semantics: a POST /api/display/draw request is accepted when
#   request_priority >= active_loader_priority
# Equal-priority requests from a different app_name are rejected while the
# previous app_name's canvas content is still visible.


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
    z_index: int | None = None


class DisplayDrawRequest(BaseModel):
    """Request for POST /api/display/draw."""

    application_name: str
    priority: int | None = None
    elements: list[DisplayElement]


class DisplayDeleteRequest(BaseModel):
    """Optional request body for selective DELETE /api/display/draw."""

    element_ids: list[str]


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
        self, app_name: str, filename: str, content: bytes, timeout: int = 10
    ) -> AssetResultResponse:
        """
        Upload an asset file.

        Args:
            app_name: Application name
            filename: Asset filename
            content: File content as bytes
            timeout: Request timeout in seconds
        """
        return self.post(
            "/api/assets/upload",
            AssetResultResponse,
            params={"application_name": app_name, "file": filename},
            data=content,
            headers={"Content-Type": "application/octet-stream"},
            timeout=timeout,
        )

    def delete_assets(self, app_name: str) -> AssetResultResponse:
        """
        Delete all assets for an application.

        Args:
            app_name: Application name
        """
        return self.delete(
            "/api/assets/upload",
            AssetResultResponse,
            params={"application_name": app_name},
        )

    # === Display ===

    def draw(
        self,
        app_name: str,
        elements: list[dict[str, Any]],
        priority: int | None = None,
    ) -> AssetResultResponse:
        """
        Draw elements to the display (raises on non-2xx).

        Args:
            app_name: Application name
            elements: List of element dictionaries
            priority: Draw priority (1–100). Defaults to server default (50).
        """
        body: dict[str, Any] = {"application_name": app_name, "elements": elements}
        if priority is not None:
            body["priority"] = priority
        return self.post(
            "/api/display/draw",
            AssetResultResponse,
            json=body,
        )

    def draw_response(
        self,
        app_name: str,
        elements: list[dict[str, Any]],
        priority: int | None = None,
    ) -> requests.Response:
        """
        Draw elements to the display and return the raw response.

        Use this variant when the call is expected to fail (400, 409, etc.)
        so that the error status code can be inspected without raising.
        """
        body: dict[str, Any] = {"application_name": app_name, "elements": elements}
        if priority is not None:
            body["priority"] = priority
        return self.post_raw("/api/display/draw", json=body)

    def draw_raw(self, data: dict[str, Any]) -> requests.Response:
        """Send a fully custom draw payload (for schema / limit testing)."""
        return self.post_raw("/api/display/draw", json=data)

    def clear_display(self) -> AssetResultResponse:
        """Clear all elements from the display."""
        return self.delete("/api/display/draw", AssetResultResponse)

    def clear_display_by_app(self, app_name: str) -> requests.Response:
        """Clear display elements belonging to a specific app_name."""
        return self.delete_raw(
            "/api/display/draw", params={"application_name": app_name}
        )

    def clear_display_elements(
        self,
        element_ids: list[str],
        app_name: str | None = None,
    ) -> AssetResultResponse:
        """Delete selected display elements and validate a successful response."""
        request = DisplayDeleteRequest(element_ids=element_ids)
        params = {"application_name": app_name} if app_name is not None else None
        return self.delete(
            "/api/display/draw",
            AssetResultResponse,
            params=params,
            json=request.model_dump(),
        )

    def clear_display_elements_response(
        self,
        element_ids: list[str],
        app_name: str | None = None,
    ) -> requests.Response:
        """Delete selected elements and return the raw response for error checks."""
        request = DisplayDeleteRequest(element_ids=element_ids)
        params = {"application_name": app_name} if app_name is not None else None
        return self.delete_raw(
            "/api/display/draw",
            params=params,
            json=request.model_dump(),
        )

    def clear_display_raw(
        self,
        data: Any,
        app_name: str | None = None,
    ) -> requests.Response:
        """Send a custom display deletion body for contract validation."""
        params = {"application_name": app_name} if app_name is not None else None
        return self.delete_raw("/api/display/draw", params=params, json=data)

    def clear_display_body_response(
        self,
        body: str,
        app_name: str | None = None,
    ) -> requests.Response:
        """Send a raw JSON string to validate display deletion parsing."""
        params = {"application_name": app_name} if app_name is not None else None
        return self.delete_raw(
            "/api/display/draw",
            params=params,
            data=body,
            headers={"Content-Type": "application/json"},
        )

    # === Audio ===

    def play_audio(
        self,
        app_name: str,
        path: str | None = None,
        *,
        stock_path: str | None = None,
    ) -> AssetResultResponse:
        """
        Play an audio file.

        Args:
            app_name: Application name
            path: Audio file path within the app's uploaded assets directory.
            stock_path: Built-in firmware sound path (e.g. "shared/volume_change.snd").

        Exactly one of `path` or `stock_path` must be provided.
        """
        if (path is None) == (stock_path is None):
            raise ValueError("Provide exactly one of path or stock_path")

        body: dict[str, str] = {"application_name": app_name}
        if path is not None:
            body["path"] = path
        else:
            body["stock_path"] = stock_path

        return self.post("/api/audio/play", AssetResultResponse, json=body)

    def stop_audio(self) -> AssetResultResponse:
        """Stop active audio playback and validate a successful response."""
        return self.delete("/api/audio/play", AssetResultResponse)

    def stop_audio_raw(self) -> requests.Response:
        """Stop audio playback and return the raw response.

        DELETE /api/audio/play returns 200 when audio was stopped and 410 when
        no audio is playing.
        """
        return self.delete_raw("/api/audio/play")
