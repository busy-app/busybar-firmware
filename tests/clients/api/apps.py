"""JavaScript applications API client and response models."""

from __future__ import annotations

from pydantic import BaseModel, Field

from .base import BaseAPI


class AppInfo(BaseModel):
    """Metadata returned for a staged or installed JavaScript app."""

    id: str = Field(min_length=1, max_length=32)
    name: str
    version: str
    author: str
    description: str
    icon_path: str
    is_debug: bool


class AppStageResponse(BaseModel):
    """Response from POST /api/apps/stage."""

    result: str
    install_key: int = Field(gt=0)
    staged: AppInfo
    installed: AppInfo | None = None


class AppListResponse(BaseModel):
    """Response from GET /api/apps/list."""

    apps: list[AppInfo]


class AppsResultResponse(BaseModel):
    """Successful install or delete response."""

    result: str


class AppsAPI(BaseAPI):
    """Typed client for JavaScript application management endpoints."""

    def list_apps(self) -> AppListResponse:
        """List installed JavaScript applications."""
        return self.get("/api/apps/list", AppListResponse)

    def stage(self, package: bytes) -> AppStageResponse:
        """Upload and validate a JavaScript application package."""
        return self.post(
            "/api/apps/stage",
            AppStageResponse,
            data=package,
            headers={"Content-Type": "application/octet-stream"},
            timeout=30,
        )

    def stage_raw(self, package: bytes):
        """Stage a package and return the raw response."""
        return self.post_raw(
            "/api/apps/stage",
            data=package,
            headers={"Content-Type": "application/octet-stream"},
            timeout=30,
        )

    def install(self, install_key: int) -> AppsResultResponse:
        """Install the currently staged app using its one-time key."""
        return self.post(
            "/api/apps/install",
            AppsResultResponse,
            params={"install_key": install_key},
            data=b"",
        )

    def install_raw(self, install_key: int | str | None = None):
        """Attempt installation and return the raw response."""
        params = (
            {"install_key": install_key}
            if install_key is not None
            else None
        )
        return self.post_raw(
            "/api/apps/install",
            params=params,
            data=b"",
        )

    def delete_app(self, app_id: str) -> AppsResultResponse:
        """Delete an installed app while preserving its settings."""
        return self.delete(
            "/api/apps",
            AppsResultResponse,
            params={"app_id": app_id},
        )

    def delete_app_raw(self, app_id: str | None = None):
        """Attempt app deletion and return the raw response."""
        params = {"app_id": app_id} if app_id is not None else None
        return self.delete_raw("/api/apps", params=params)
