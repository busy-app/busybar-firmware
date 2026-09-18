"""JavaScript applications API client and response models."""

from __future__ import annotations

from typing import Any

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


class AppSettingsDocument(BaseModel):
    """Complete settings document returned by an installed application."""

    version: int = Field(gt=0)
    values: dict[str, Any]


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

    def get_settings(self, app_id: str) -> AppSettingsDocument:
        """Read an application's complete settings document."""
        return self.get(
            "/api/apps/settings",
            AppSettingsDocument,
            params={"app_id": app_id},
        )

    def get_settings_raw(self, app_id: str | None = None):
        """Read settings and return the raw response."""
        params = {"app_id": app_id} if app_id is not None else None
        return self.get_raw("/api/apps/settings", params=params)

    def set_settings(
        self,
        app_id: str,
        document: AppSettingsDocument | dict[str, Any],
    ) -> AppsResultResponse:
        """Replace an application's settings document."""
        body = (
            document.model_dump()
            if isinstance(document, AppSettingsDocument)
            else document
        )
        return self.put(
            "/api/apps/settings",
            AppsResultResponse,
            params={"app_id": app_id},
            json=body,
        )

    def set_settings_raw(
        self,
        app_id: str | None = None,
        document: dict[str, Any] | None = None,
        *,
        body: bytes | str | None = None,
    ):
        """Attempt to replace settings and return the raw response."""
        params = {"app_id": app_id} if app_id is not None else None
        if body is not None:
            return self.put_raw(
                "/api/apps/settings",
                params=params,
                data=body,
                headers={"Content-Type": "application/json"},
            )
        return self.put_raw(
            "/api/apps/settings",
            params=params,
            json=document,
        )

    def reset_settings(self, app_id: str) -> AppsResultResponse:
        """Reset an application's settings to schema defaults."""
        return self.delete(
            "/api/apps/settings",
            AppsResultResponse,
            params={"app_id": app_id},
        )

    def reset_settings_raw(self, app_id: str | None = None):
        """Attempt to reset settings and return the raw response."""
        params = {"app_id": app_id} if app_id is not None else None
        return self.delete_raw("/api/apps/settings", params=params)
