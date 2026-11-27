"""Application packages with independently controlled download completion."""

from __future__ import annotations

import http.server
import threading
import uuid
from dataclasses import dataclass, field
from http import HTTPStatus
from urllib.parse import unquote, urlsplit

from utils.fetch_http_server import FetchHTTPServer


DOWNLOAD_START_TIMEOUT_S = 10
DOWNLOAD_HOLD_TIMEOUT_S = 60


@dataclass
class AppPackageDownload:
    url: str
    body: bytes
    status: int
    is_truncated: bool = False
    has_content_length: bool = True
    request_target: str | None = None
    requested: threading.Event = field(default_factory=threading.Event)
    release: threading.Event = field(default_factory=threading.Event)

    def wait_started(self) -> None:
        assert self.requested.wait(
            DOWNLOAD_START_TIMEOUT_S
        ), f"Device did not start downloading {self.url!r}"


class AppPackageServer(FetchHTTPServer):
    """Serve in-memory packages using the existing host-server network setup."""

    def __init__(self, host: str):
        self.packages: dict[str, AppPackageDownload] = {}
        super().__init__((host, 0), AppPackageRequestHandler)

    def add_package(
        self,
        body: bytes,
        *,
        held: bool = False,
        status: int = HTTPStatus.OK,
        truncated: bool = False,
        filename: str | None = None,
        known_length: bool = True,
    ) -> AppPackageDownload:
        path = f"/apps/{uuid.uuid4().hex}/{filename or 'app.tgz'}"
        download = AppPackageDownload(
            self.url(path), body, status, truncated, known_length
        )
        if not held:
            download.release.set()
        self.packages[path] = download
        return download

    def release_all(self) -> None:
        for download in self.packages.values():
            download.release.set()


class AppPackageRequestHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self) -> None:
        download = self.server.packages.get(unquote(urlsplit(self.path).path))
        if download is None:
            self.send_error(HTTPStatus.NOT_FOUND)
            return

        self.server.requests.put({"method": self.command, "path": self.path})
        download.request_target = self.path
        try:
            self.send_response(download.status)
            self.send_header("Content-Type", "application/octet-stream")
            if download.has_content_length:
                self.send_header("Content-Length", str(len(download.body)))
            self.end_headers()
            split_at = len(download.body) // 2
            self.wfile.write(download.body[:split_at])
            self.wfile.flush()
            download.requested.set()
            if download.is_truncated:
                return
            # Hold an incomplete body so cancellation happens during download,
            # before unpacking can change the installed application.
            if download.release.wait(DOWNLOAD_HOLD_TIMEOUT_S):
                self.wfile.write(download.body[split_at:])
                self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
            pass
