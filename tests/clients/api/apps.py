"""JavaScript applications API client and response models."""

from __future__ import annotations

from typing import Annotated, Any, Literal

from pydantic import BaseModel, Field, model_validator

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
    """Successful JavaScript application management response."""

    result: str


class AppSettingsDocument(BaseModel):
    """Complete settings document returned by an installed application."""

    version: int = Field(gt=0)
    values: dict[str, Any]


AppQueueAppId = Annotated[str, Field(pattern=r"^[a-z0-9_\-][a-z0-9_\-.]{0,31}$")]
AppQueueItemId = Annotated[int, Field(strict=True, ge=0, le=(1 << 32) - 1)]
AppQueueOperationType = Literal["install", "remove"]
AppQueueError = Literal["timeout", "queue_full", "skipped"]


class AppQueueOperation(BaseModel):
    """Operation metadata returned by the application queue."""

    type: AppQueueOperationType
    version: str | None = None


class AppQueueRequestOperation(AppQueueOperation):
    """Queue command; install operations require a download URL."""

    url: str | None = None

    @model_validator(mode="after")
    def validate_install_url(self) -> AppQueueRequestOperation:
        if self.type == "install" and self.url is None:
            raise ValueError("An install operation requires url")
        return self


class AppQueueRequestItem(BaseModel):
    """Application and operation to enqueue."""

    app_id: AppQueueAppId
    operation: AppQueueRequestOperation


class AppQueueAddRequest(BaseModel):
    """POST /api/apps/queue body containing one to 100 operations."""

    operations: list[AppQueueRequestItem] = Field(min_length=1, max_length=100)


class AppQueueResponseItem(BaseModel):
    """Enqueue outcome with exactly one of item_id or error."""

    app_id: AppQueueAppId
    operation: AppQueueOperation
    item_id: AppQueueItemId | None = None
    error: AppQueueError | None = None

    @model_validator(mode="after")
    def validate_outcome(self) -> AppQueueResponseItem:
        has_item_id = "item_id" in self.model_fields_set
        has_error = "error" in self.model_fields_set
        if has_item_id == has_error:
            raise ValueError("Expected exactly one of item_id or error")
        if (has_item_id and self.item_id is None) or (has_error and self.error is None):
            raise ValueError("Queue outcome must not be null")
        return self


class AppQueueAddResponse(BaseModel):
    """Per-operation enqueue outcomes returned with HTTP 202."""

    result: list[AppQueueResponseItem]


class AppQueueDetail(BaseModel):
    """Optional step and download percentage for an active operation."""

    step: str
    progress: Annotated[int, Field(strict=True, ge=0, le=100)] | None = None


class AppQueueItem(BaseModel):
    """Active or pending operation returned by GET /api/apps/queue."""

    item_id: AppQueueItemId
    app_id: AppQueueAppId
    operation: AppQueueOperation
    status: Literal["pending", "in_progress"]
    detail: AppQueueDetail | None = None


class AppQueueGetResponse(BaseModel):
    """Snapshot of the active and pending application operations."""

    result: list[AppQueueItem]


class AppQueueCancelRequest(BaseModel):
    """Cancel one operation by its queue ID."""

    item_id: AppQueueItemId


class AppQueueClearRequest(BaseModel):
    """Cancel all active and pending operations."""

    all: Literal[True] = True


class AppsAPI(BaseAPI):
    """Typed client for JavaScript application management endpoints."""

    def list_apps(self) -> AppListResponse:
        """List installed JavaScript applications."""
        return self.get("/api/apps/list", AppListResponse)

    def get_queue(self) -> AppQueueGetResponse:
        """Read active and pending application operations."""
        return self.get("/api/apps/queue", AppQueueGetResponse)

    def add_to_queue(self, request: AppQueueAddRequest) -> AppQueueAddResponse:
        """Enqueue a batch and validate its HTTP 202 response."""
        response = self.post_raw(
            "/api/apps/queue", json=request.model_dump(exclude_none=True)
        )
        self.assert_status(response, 202)
        return self._validate(response, AppQueueAddResponse)

    def cancel_queue(
        self, request: AppQueueCancelRequest | AppQueueClearRequest
    ) -> AppsResultResponse:
        """Request cancellation and validate the HTTP 202 acknowledgement."""
        response = self.cancel_queue_raw(request)
        self.assert_status(response, 202)
        return self._validate(response, AppsResultResponse)

    def cancel_queue_raw(self, request: AppQueueCancelRequest | AppQueueClearRequest):
        """Cancel an operation or clear the queue, returning the raw response."""
        return self.post_raw("/api/apps/queue/cancel", json=request.model_dump())

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

    def launch(self, app_id: str) -> AppsResultResponse:
        """Launch an installed JavaScript application."""
        return self.post(
            "/api/apps/launch",
            AppsResultResponse,
            params={"app_id": app_id},
            data=b"",
        )

    def launch_raw(self, app_id: str | None = None):
        """Attempt to launch an application and return the raw response."""
        params = {"app_id": app_id} if app_id is not None else None
        return self.post_raw(
            "/api/apps/launch",
            params=params,
            data=b"",
        )

    def quit(self) -> AppsResultResponse:
        """Quit the currently running JavaScript application."""
        return self.post(
            "/api/apps/quit",
            AppsResultResponse,
            data=b"",
        )

    def quit_raw(self):
        """Attempt to quit the current application and return the response."""
        return self.post_raw("/api/apps/quit", data=b"")

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
