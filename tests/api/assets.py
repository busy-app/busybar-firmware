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

from pydantic import BaseModel

from .base import BaseAPI


# === Response Models ===


class AssetResultResponse(BaseModel):
    """Generic asset/display/audio operation result."""

    result: str


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

    def draw(self, app_id: str, elements: list[dict[str, Any]]) -> AssetResultResponse:
        """
        Draw elements to the display.

        Args:
            app_id: Application ID
            elements: List of element dictionaries
        """
        return self.post(
            "/api/display/draw",
            AssetResultResponse,
            json={"app_id": app_id, "elements": elements},
        )

    def draw_raw(self, data: dict[str, Any]):
        """Send raw draw command (for error testing)."""
        return self.post_raw("/api/display/draw", json=data)

    def clear_display(self) -> AssetResultResponse:
        """Clear the display."""
        return self.delete("/api/display/draw", AssetResultResponse)

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
