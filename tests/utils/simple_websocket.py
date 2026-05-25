from __future__ import annotations

import base64
import hashlib
import os
import socket
import ssl
import struct
from dataclasses import dataclass
from urllib.parse import urlparse, urlunparse


_WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"


@dataclass
class WebSocketUpgradeResult:
    status_code: int
    headers: dict[str, str]


@dataclass
class WebSocketFrame:
    opcode: int
    payload: bytes


def _target_from_url(url: str) -> tuple[str, int, str, str]:
    parsed = urlparse(url)
    if parsed.scheme not in {"ws", "wss", "http", "https"}:
        raise ValueError(f"Unsupported WebSocket URL scheme: {parsed.scheme}")

    host = parsed.hostname
    if not host:
        raise ValueError(f"Missing host in WebSocket URL: {url}")

    scheme = {"http": "ws", "https": "wss"}.get(parsed.scheme, parsed.scheme)
    port = parsed.port or (443 if scheme == "wss" else 80)
    path = parsed.path or "/"
    if parsed.query:
        path = f"{path}?{parsed.query}"
    return host, port, path, scheme


def websocket_url(web_base_url: str, path: str) -> str:
    parsed = urlparse(web_base_url)
    scheme = "wss" if parsed.scheme in {"https", "wss"} else "ws"
    return urlunparse((scheme, parsed.netloc, path, "", "", ""))


def _create_connection(host: str, port: int, scheme: str, timeout: float) -> socket.socket:
    sock = socket.create_connection((host, port), timeout=timeout)
    sock.settimeout(timeout)
    if scheme != "wss":
        return sock
    try:
        return ssl.create_default_context().wrap_socket(sock, server_hostname=host)
    except Exception:
        sock.close()
        raise


def _read_headers(sock: socket.socket) -> bytes:
    data = bytearray()
    while b"\r\n\r\n" not in data:
        chunk = sock.recv(4096)
        if not chunk:
            break
        data.extend(chunk)
        if len(data) > 65536:
            raise RuntimeError("WebSocket handshake headers exceeded 64 KiB")
    return bytes(data)


def _parse_handshake_response(raw: bytes) -> WebSocketUpgradeResult:
    header_block = raw.split(b"\r\n\r\n", 1)[0].decode("iso-8859-1", errors="replace")
    lines = header_block.split("\r\n")
    status_parts = lines[0].split(" ", 2)
    status_code = int(status_parts[1]) if len(status_parts) > 1 else 0

    headers: dict[str, str] = {}
    for line in lines[1:]:
        if ":" not in line:
            continue
        key, value = line.split(":", 1)
        headers[key.lower()] = value.strip()

    return WebSocketUpgradeResult(status_code=status_code, headers=headers)


def websocket_upgrade(url: str, timeout: float = 5.0) -> WebSocketUpgradeResult:
    host, port, path, scheme = _target_from_url(url)
    key = base64.b64encode(os.urandom(16)).decode("ascii")
    request = (
        f"GET {path} HTTP/1.1\r\n"
        f"Host: {host}:{port}\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        f"Sec-WebSocket-Key: {key}\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "\r\n"
    ).encode("ascii")

    with _create_connection(host, port, scheme, timeout) as sock:
        sock.sendall(request)
        return _parse_handshake_response(_read_headers(sock))


class SimpleWebSocket:
    def __init__(self, url: str, timeout: float = 5.0):
        self.url = url
        self.timeout = timeout
        self.sock: socket.socket | None = None

    def __enter__(self) -> "SimpleWebSocket":
        self.connect()
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    def connect(self) -> None:
        host, port, path, scheme = _target_from_url(self.url)
        key = base64.b64encode(os.urandom(16)).decode("ascii")
        request = (
            f"GET {path} HTTP/1.1\r\n"
            f"Host: {host}:{port}\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            f"Sec-WebSocket-Key: {key}\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            "\r\n"
        ).encode("ascii")

        sock = _create_connection(host, port, scheme, self.timeout)
        try:
            sock.sendall(request)
            result = _parse_handshake_response(_read_headers(sock))
            if result.status_code != 101:
                raise RuntimeError(f"WebSocket upgrade failed with HTTP {result.status_code}")

            expected_accept = base64.b64encode(
                hashlib.sha1(f"{key}{_WS_GUID}".encode("ascii")).digest()
            ).decode("ascii")
            actual_accept = result.headers.get("sec-websocket-accept")
            if actual_accept != expected_accept:
                raise RuntimeError("WebSocket upgrade returned invalid Sec-WebSocket-Accept")
        except Exception:
            sock.close()
            raise

        self.sock = sock

    def close(self) -> None:
        if self.sock:
            try:
                self._send_frame(0x8, b"")
            except OSError:
                pass
            self.sock.close()
            self.sock = None

    def send_text(self, text: str) -> None:
        self._send_frame(0x1, text.encode("utf-8"))

    def send_pong(self, payload: bytes = b"") -> None:
        self._send_frame(0xA, payload)

    def recv_frame(self, timeout: float | None = None) -> WebSocketFrame:
        if not self.sock:
            raise RuntimeError("WebSocket is not connected")

        old_timeout = self.sock.gettimeout()
        if timeout is not None:
            self.sock.settimeout(timeout)
        try:
            first = self._recv_exact(2)
            opcode = first[0] & 0x0F
            masked = bool(first[1] & 0x80)
            length = first[1] & 0x7F

            if length == 126:
                length = struct.unpack("!H", self._recv_exact(2))[0]
            elif length == 127:
                length = struct.unpack("!Q", self._recv_exact(8))[0]

            mask = self._recv_exact(4) if masked else b""
            payload = self._recv_exact(length) if length else b""
            if masked:
                payload = bytes(byte ^ mask[i % 4] for i, byte in enumerate(payload))

            return WebSocketFrame(opcode=opcode, payload=payload)
        finally:
            if timeout is not None:
                self.sock.settimeout(old_timeout)

    def recv_data_frame(self, timeout: float = 5.0) -> WebSocketFrame:
        while True:
            frame = self.recv_frame(timeout=timeout)
            if frame.opcode in (0x1, 0x2):
                return frame
            if frame.opcode == 0x9:
                self.send_pong(frame.payload)
                continue
            if frame.opcode == 0x8:
                raise RuntimeError("WebSocket closed by peer")

    def _send_frame(self, opcode: int, payload: bytes) -> None:
        if not self.sock:
            raise RuntimeError("WebSocket is not connected")

        header = bytearray([0x80 | opcode])
        length = len(payload)
        if length < 126:
            header.append(0x80 | length)
        elif length < (1 << 16):
            header.append(0x80 | 126)
            header.extend(struct.pack("!H", length))
        else:
            header.append(0x80 | 127)
            header.extend(struct.pack("!Q", length))

        mask = os.urandom(4)
        masked_payload = bytes(byte ^ mask[i % 4] for i, byte in enumerate(payload))
        self.sock.sendall(bytes(header) + mask + masked_payload)

    def _recv_exact(self, length: int) -> bytes:
        if not self.sock:
            raise RuntimeError("WebSocket is not connected")

        data = bytearray()
        while len(data) < length:
            chunk = self.sock.recv(length - len(data))
            if not chunk:
                raise RuntimeError("WebSocket closed while reading frame")
            data.extend(chunk)
        return bytes(data)
