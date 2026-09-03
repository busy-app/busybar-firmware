"""
Storage API client and Pydantic models.

Endpoints:
- GET /api/storage/status
- GET /api/storage/list
- GET /api/storage/read
- POST /api/storage/write
- POST /api/storage/mkdir
- POST /api/storage/rename
- DELETE /api/storage/remove
"""

from __future__ import annotations

from typing import Literal

from pydantic import BaseModel, field_validator

from .base import BaseAPI


# === Response Models ===


class StorageStatusResponse(BaseModel):
    """Response from GET /api/storage/status."""

    used_bytes: int
    free_bytes: int
    total_bytes: int


class StorageItem(BaseModel):
    """Single item in storage listing."""

    type: Literal["file", "dir"]
    name: str
    size: int | None = None  # Only for files


class StorageListResponse(BaseModel):
    """Response from GET /api/storage/list."""

    list: list[StorageItem]


class StorageResultResponse(BaseModel):
    """Generic storage operation result."""

    result: str

    @field_validator("result")
    @classmethod
    def validate_result(cls, v: str) -> str:
        """Validate result indicates success."""
        assert v, "Expected non-empty result"
        return v


# === API Client ===


class StorageAPI(BaseAPI):
    """
    Storage API client.

    Endpoints:
    - GET /api/storage/status - Get storage usage
    - GET /api/storage/list - List directory contents
    - GET /api/storage/read - Read file contents
    - POST /api/storage/write - Write file
    - POST /api/storage/mkdir - Create directory
    - POST /api/storage/rename - Rename/move file
    - DELETE /api/storage/remove - Remove file/directory
    """

    def get_status(self) -> StorageStatusResponse:
        """Get storage usage statistics."""
        return self.get("/api/storage/status", StorageStatusResponse)

    def list(self, path: str) -> StorageListResponse:
        """
        List contents of a directory.

        Args:
            path: Directory path (e.g., "/ext")
        """
        return self.get("/api/storage/list", StorageListResponse, params={"path": path})

    def read(self, path: str):
        """
        Read file contents.

        Args:
            path: File path

        Returns:
            Raw response with file content in response.content
        """
        return self.get_raw("/api/storage/read", params={"path": path})

    def write(
        self,
        path: str,
        content: bytes,
        timeout: int = 10,
        append: bool | None = None,
    ):
        """
        Write file to storage.

        Args:
            path: File path
            content: File content as bytes
            timeout: Request timeout in seconds
            append: Append to the file instead of replacing it.
                None omits the parameter, True sends append=1,
                False sends an explicit append=0.

        Returns:
            Raw response (for checking status codes)
        """
        params = {"path": path}
        if append is not None:
            params["append"] = "1" if append else "0"
        return self.post_raw(
            "/api/storage/write",
            params=params,
            data=content,
            headers={"Content-Type": "application/octet-stream"},
            timeout=timeout,
        )

    def mkdir(self, path: str) -> StorageResultResponse:
        """
        Create a directory.

        Args:
            path: Directory path to create
        """
        return self.post("/api/storage/mkdir", StorageResultResponse, params={"path": path}, data=b"")

    def rename(self, path: str, new_path: str) -> StorageResultResponse:
        """
        Rename/move a file.

        Args:
            path: Current file path
            new_path: New file path
        """
        return self.post(
            "/api/storage/rename",
            StorageResultResponse,
            params={"path": path, "new_path": new_path},
            data=b"",
        )

    def rename_raw(self, path: str, new_path: str):
        """Rename file and return raw response (for error testing)."""
        return self.post_raw(
            "/api/storage/rename",
            params={"path": path, "new_path": new_path},
            data=b"",
        )

    def remove(self, path: str):
        """
        Remove a file or directory.

        Args:
            path: Path to remove

        Returns:
            Dict response (for cleanup operations)
        """
        return self.delete("/api/storage/remove", params={"path": path})

    def remove_raw(self, path: str):
        """
        Remove a file or directory and return raw response.

        Args:
            path: Path to remove

        Returns:
            Raw response (for status code checking)
        """
        return self.delete_raw("/api/storage/remove", params={"path": path})
