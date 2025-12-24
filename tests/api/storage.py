"""
Storage API client and Pydantic models.

Endpoints:
- GET /api/storage/status
- GET /api/storage/list
- GET /api/storage/read
- POST /api/storage/write
- POST /api/storage/mkdir
- DELETE /api/storage/remove
"""

from __future__ import annotations

from typing import Literal

from pydantic import BaseModel

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

    def write(self, path: str, content: bytes, timeout: int = 10):
        """
        Write file to storage.

        Args:
            path: File path
            content: File content as bytes
            timeout: Request timeout in seconds

        Returns:
            Raw response (for checking status codes)
        """
        return self.post_raw(
            "/api/storage/write",
            params={"path": path},
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
        return self.post("/api/storage/mkdir", StorageResultResponse, params={"path": path})

    def remove(self, path: str):
        """
        Remove a file or directory.

        Args:
            path: Path to remove

        Returns:
            Raw response (for cleanup operations)
        """
        return self.delete("/api/storage/remove", params={"path": path})
