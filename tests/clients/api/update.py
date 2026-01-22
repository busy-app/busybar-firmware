"""
Update API client and Pydantic models.

Endpoints:
- POST /api/update
- GET /api/update/status
- POST /api/update/check
- GET /api/update/changelog
- POST /api/update/install
- POST /api/update/abort_download
"""

from __future__ import annotations

from typing import Literal

from pydantic import BaseModel, field_validator

from .base import BaseAPI


# === Response Models ===


class DownloadProgress(BaseModel):
    """Download progress info in install status."""

    speed_bytes_per_sec: int
    received_bytes: int
    total_bytes: int


class InstallStatus(BaseModel):
    """Install status section of update status response."""

    is_allowed: bool
    event: Literal[
        "session_start", "session_stop", "action_begin",
        "action_done", "detail_change", "action_progress", "none"
    ]
    action: Literal[
        "download", "sha_verification", "unpack",
        "prepare", "apply", "none"
    ]
    status: Literal[
        "ok", "battery_low", "busy", "download_failure",
        "download_abort", "sha_mismatch", "unpack_staging_dir_failure",
        "unpack_archive_open_failure", "unpack_archive_unpack_failure",
        "install_manifest_not_found", "install_manifest_invalid",
        "install_session_config_failure", "install_pointer_setup_failure",
        "unknown_failure"
    ]
    detail: str | None = None
    download: DownloadProgress | None = None

    # === Status Check Properties ===

    @property
    def is_idle(self) -> bool:
        """Check if no install session is active."""
        return self.event in ("session_stop", "none")

    @property
    def is_in_progress(self) -> bool:
        """Check if install is in progress."""
        return self.event in ("session_start", "action_begin", "action_progress", "detail_change")

    @property
    def is_downloading(self) -> bool:
        """Check if download is in progress."""
        return self.is_in_progress and self.action == "download"

    @property
    def is_verifying(self) -> bool:
        """Check if SHA verification is in progress."""
        return self.is_in_progress and self.action == "sha_verification"

    @property
    def is_unpacking(self) -> bool:
        """Check if unpacking is in progress."""
        return self.is_in_progress and self.action == "unpack"

    @property
    def is_applying(self) -> bool:
        """Check if applying update is in progress."""
        return self.is_in_progress and self.action == "apply"

    @property
    def is_ok(self) -> bool:
        """Check if last operation succeeded."""
        return self.status == "ok"

    @property
    def is_failed(self) -> bool:
        """Check if last operation failed."""
        return self.status not in ("ok", "busy")

    @property
    def failure_reason(self) -> str | None:
        """Get failure reason if failed, None otherwise."""
        if self.is_failed:
            return self.status
        return None


class CheckStatus(BaseModel):
    """Check status section of update status response."""

    available_version: str | None = None
    event: Literal["start", "stop", "none"]
    result: Literal["available", "not_available", "failure", "none"]

    # === Status Check Properties ===

    @property
    def is_checking(self) -> bool:
        """Check if update check is in progress."""
        return self.event == "start"

    @property
    def is_available(self) -> bool:
        """Check if update is available."""
        return self.result == "available" and bool(self.available_version)

    @property
    def is_up_to_date(self) -> bool:
        """Check if firmware is up to date (no update available)."""
        return self.result == "not_available"

    @property
    def has_failed(self) -> bool:
        """Check if update check failed."""
        return self.result == "failure"


class UpdateStatusResponse(BaseModel):
    """Response from GET /api/update/status."""

    install: InstallStatus
    check: CheckStatus


class UpdateResultResponse(BaseModel):
    """Generic update operation result."""

    result: str

    @field_validator("result")
    @classmethod
    def validate_result(cls, v: str) -> str:
        """Validate result indicates success."""
        assert v, "Expected non-empty result"
        return v


# === API Client ===


class UpdateAPI(BaseAPI):
    """
    Update API client.

    Endpoints:
    - POST /api/update - Upload firmware update package
    - GET /api/update/status - Get update status
    - POST /api/update/check - Start update check
    - GET /api/update/changelog - Get changelog for version
    - POST /api/update/install - Install firmware version
    - POST /api/update/abort_download - Abort download
    """

    def upload_package(self, content: bytes, timeout: int = 30):
        """
        Upload firmware update package.

        Args:
            content: Update package content
            timeout: Request timeout in seconds

        Returns:
            Raw response
        """
        return self.post_raw(
            "/api/update",
            data=content,
            headers={"Content-Type": "application/octet-stream"},
            timeout=timeout,
        )

    def get_status(self) -> UpdateStatusResponse:
        """Get current update status."""
        return self.get("/api/update/status", UpdateStatusResponse)

    def check(self) -> UpdateResultResponse:
        """Start update check."""
        return self.post("/api/update/check", UpdateResultResponse)

    def check_raw(self):
        """Start update check and return raw response (for status code checking)."""
        return self.post_raw("/api/update/check")

    def get_changelog(self, version: str):
        """
        Get changelog for a specific version.

        Args:
            version: Firmware version

        Returns:
            Raw response
        """
        return self.get_raw("/api/update/changelog", params={"version": version})

    def get_changelog_raw(self):
        """Get changelog without version parameter (for error testing)."""
        return self.get_raw("/api/update/changelog")

    def install(self, version: str):
        """
        Install a specific firmware version.

        Args:
            version: Firmware version to install

        Returns:
            Raw response
        """
        return self.post_raw("/api/update/install", params={"version": version})

    def install_raw(self):
        """Install without version parameter (for error testing)."""
        return self.post_raw("/api/update/install")

    def abort_download(self) -> UpdateResultResponse:
        """Abort ongoing download."""
        return self.post("/api/update/abort_download", UpdateResultResponse)

    # === Status Check Helpers ===

    def is_update_available(self) -> bool:
        """Check if firmware update is available."""
        return self.get_status().check.is_available

    def is_up_to_date(self) -> bool:
        """Check if firmware is up to date."""
        return self.get_status().check.is_up_to_date

    def is_install_in_progress(self) -> bool:
        """Check if install is in progress."""
        return self.get_status().install.is_in_progress

    def is_idle(self) -> bool:
        """Check if updater is idle (no operations in progress)."""
        status = self.get_status()
        return status.install.is_idle and not status.check.is_checking

    def get_available_version(self) -> str | None:
        """Get available update version, or None if not available."""
        status = self.get_status()
        if status.check.is_available:
            return status.check.available_version
        return None

    def wait_for_idle(self, timeout: int = 60, poll_interval: float = 1.0) -> UpdateStatusResponse:
        """
        Wait for updater to become idle.

        Args:
            timeout: Maximum wait time in seconds
            poll_interval: Time between status checks

        Returns:
            Final status response

        Raises:
            TimeoutError: If timeout is reached
        """
        import time
        start = time.time()
        while time.time() - start < timeout:
            status = self.get_status()
            if status.install.is_idle and not status.check.is_checking:
                return status
            time.sleep(poll_interval)
        raise TimeoutError(f"Updater did not become idle within {timeout}s")

    def wait_for_check_complete(self, timeout: int = 30, poll_interval: float = 1.0) -> CheckStatus:
        """
        Wait for update check to complete.

        Args:
            timeout: Maximum wait time in seconds
            poll_interval: Time between status checks

        Returns:
            Check status after completion

        Raises:
            TimeoutError: If timeout is reached
        """
        import time
        start = time.time()
        while time.time() - start < timeout:
            status = self.get_status()
            if not status.check.is_checking:
                return status.check
            time.sleep(poll_interval)
        raise TimeoutError(f"Update check did not complete within {timeout}s")
