"""Deterministic HTTP server shared by device-side Fetch integration tests."""

import http.server
import json
import queue
import socketserver
import ssl
import threading
import time
from dataclasses import dataclass
from http import HTTPStatus
from urllib.parse import parse_qs, urlsplit


KNOWN_PAYLOAD = b"busybar-fetch-test\n" * 10
TEXT_PAYLOAD = b"busybar-js-fetch-text\n"
UNICODE_TEXT = "\u041f\u0440\u0438\u0432\u0435\u0442, BusyBar\n"
UNICODE_TEXT_PAYLOAD = UNICODE_TEXT.encode("utf-8")
NON_BMP_UNICODE_TEXT = "\u041f\u0440\u0438\u0432\u0435\u0442, BusyBar \U0001f44b\n"
NON_BMP_UNICODE_TEXT_PAYLOAD = NON_BMP_UNICODE_TEXT.encode("utf-8")
EMPTY_PAYLOAD = b""
JSON_PAYLOAD = b'{"message":"busybar-js-fetch","value":42}'
INVALID_JSON_PAYLOAD = b'{"message":"unterminated"'
INVALID_UTF8_PAYLOAD = b"busybar-\xff\n"
HEADERS_PAYLOAD = b"headers-ok\n"
CREATED_PAYLOAD = b'{"created":true}'
SERVER_ERROR_PAYLOAD = b'{"error":"server-error"}'
TRUNCATED_PAYLOAD = b"truncated-fetch-test\n" * 3
UNKNOWN_LENGTH_PAYLOAD = b"close-delimited-fetch-payload\n" * 7
NOT_FOUND_PAYLOAD = b"fetch-route-not-found\n"
REQUEST_RESPONSE = b"request-captured\n"
EMPTY_HEADER_RESPONSE_STATUSES = {
    "/empty-headers": 200,
    "/empty-headers-unauthorized": 401,
    "/empty-headers-no-content": 204,
}
EMPTY_RESPONSE_HEADERS = (
    b"Server: \r\n" b"X-Empty:\r\n" b"X-Tab:\t\r\n" b"X-Whitespace: \t \r\n"
)
MALFORMED_RESPONSE_HEADERS = {
    "/invalid-headers-empty-name": b": value\r\n",
    "/invalid-headers-empty-value-lf": b"X-Bad:\n",
    "/invalid-headers-whitespace-value-lf": b"X-Bad: \t\n",
    "/invalid-headers-stall": b"X-Bad:\n",
}
HEADER_DISCONNECT_TIMEOUT_S = 5
FETCH_HTTP11_TIMEOUT_S = 12
FETCH_RESPONSE_HOLD_S = 15
FETCH_PACKET_INTERVAL_S = 0.025
FETCH_CONNECTION_POLL_S = 0.1
HTTP11_INCOMPLETE_TRAILER_CASES = frozenset(
    {
        "missing-final-crlf",
        "truncated-final-crlf",
        "truncated-trailer",
        "missing-trailer-terminator",
    }
)
CHUNKED_BINARY_PAYLOAD = bytes(range(256)) * 4
CHUNKED_JSON_PAYLOAD = json.dumps(
    {"message": NON_BMP_UNICODE_TEXT, "value": 42}, ensure_ascii=False
).encode("utf-8")


@dataclass(frozen=True)
class FetchHTTP11Response:
    packets: tuple[bytes, ...]
    payload: bytes = b""
    headers: tuple[tuple[str, str], ...] = (("Transfer-Encoding", "chunked"),)
    expected_error: str | None = None
    keep_open: bool = False
    packet_interval_s: float = 0


def _encode_chunks(chunks):
    return (
        b"".join(
            f"{len(chunk):x}\r\n".encode("ascii") + chunk + b"\r\n" for chunk in chunks
        )
        + b"0\r\n\r\n"
    )


HTTP11_SUCCESS_RESPONSES = {
    "simple": FetchHTTP11Response((b"5\r\nhello\r\n0\r\n\r\n",), b"hello"),
    "empty": FetchHTTP11Response((b"0\r\n\r\n",)),
    "multiple": FetchHTTP11Response(
        (b"5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n",), b"hello world"
    ),
    "uppercase-size": FetchHTTP11Response(
        (b"1B\r\nhello from chunked transfer\r\n0\r\n\r\n",),
        b"hello from chunked transfer",
    ),
    "extensions": FetchHTTP11Response(
        (b'5; name="a;b"; flag\r\nhello\r\n0; last=yes\r\n\r\n',), b"hello"
    ),
    "trailers": FetchHTTP11Response(
        (b"5\r\nhello\r\n0\r\nX-Checksum: value\r\nX-Empty:\r\n\r\n",), b"hello"
    ),
    "bytewise": FetchHTTP11Response(
        tuple(bytes((value,)) for value in b"a\r\n0123456789\r\n0\r\n\r\n"),
        b"0123456789",
        packet_interval_s=FETCH_PACKET_INTERVAL_S,
    ),
    "split-size-and-trailers": FetchHTTP11Response(
        (
            b"1",
            b"b; x=1\r",
            b"\nhello from chunked transfer\r",
            b"\n0\r",
            b"\nX-End: y\r",
            b"\n\r",
            b"\n",
        ),
        b"hello from chunked transfer",
        packet_interval_s=FETCH_PACKET_INTERVAL_S,
    ),
    "binary": FetchHTTP11Response(
        (_encode_chunks((CHUNKED_BINARY_PAYLOAD,)),), CHUNKED_BINARY_PAYLOAD
    ),
    "many-small-chunks": FetchHTTP11Response(
        (
            _encode_chunks(
                CHUNKED_BINARY_PAYLOAD[i : i + 4]
                for i in range(0, len(CHUNKED_BINARY_PAYLOAD), 4)
            ),
        ),
        CHUNKED_BINARY_PAYLOAD,
    ),
    "utf8-and-json": FetchHTTP11Response(
        (_encode_chunks(bytes((value,)) for value in CHUNKED_JSON_PAYLOAD),),
        CHUNKED_JSON_PAYLOAD,
    ),
    "mixed-case-encoding": FetchHTTP11Response(
        (b"5\r\nhello\r\n0\r\n\r\n",),
        b"hello",
        headers=(("tRaNsFeR-EnCoDiNg", "ChUnKeD"),),
    ),
    "keep-alive": FetchHTTP11Response(
        (b"5\r\nhello\r\n0\r\n\r\n",), b"hello", keep_open=True
    ),
    "known-length": FetchHTTP11Response(
        (KNOWN_PAYLOAD,),
        KNOWN_PAYLOAD,
        headers=(("Content-Length", str(len(KNOWN_PAYLOAD))),),
        keep_open=True,
    ),
    "zero-length": FetchHTTP11Response(
        (), headers=(("Content-Length", "0"),), keep_open=True
    ),
    "close-delimited": FetchHTTP11Response(
        (UNKNOWN_LENGTH_PAYLOAD,), UNKNOWN_LENGTH_PAYLOAD, headers=()
    ),
}

HTTP11_ERROR_RESPONSES = {
    name: FetchHTTP11Response((packet,), expected_error="Chunked transfer error")
    for name, packet in (
        ("invalid-size", b"Z\r\nhello\r\n0\r\n\r\n"),
        ("missing-size", b"\r\nhello\r\n0\r\n\r\n"),
        ("size-overflow", b"10000000000000000\r\n"),
        ("invalid-size-lf", b"1\rX"),
        ("invalid-data-cr", b"1\r\naX\n0\r\n\r\n"),
        ("invalid-data-lf", b"1\r\na\rX0\r\n\r\n"),
    )
}
HTTP11_ERROR_RESPONSES.update(
    {
        name: FetchHTTP11Response((packet,), expected_error="Incomplete response body")
        for name, packet in (
            ("truncated-size", b"1"),
            ("truncated-size-crlf", b"1\r"),
            ("maximum-size-truncated", b"FFFFFFFFFFFFFFFF\r\n"),
            ("truncated-data", b"5\r\nhel"),
            ("truncated-data-cr", b"1\r\na"),
            ("truncated-data-lf", b"1\r\na\r"),
            ("missing-last-chunk", b"1\r\na\r\n"),
            ("missing-final-crlf", b"1\r\na\r\n0\r\n"),
            ("truncated-final-crlf", b"0\r\n\r"),
            ("truncated-trailer", b"0\r\nX-Trailer: value"),
            ("missing-trailer-terminator", b"0\r\nX-Trailer: value\r\n"),
        )
    }
)

HTTP11_ERROR_RESPONSES.update(
    {
        name: FetchHTTP11Response(
            (b"5\r\nhello\r\n0\r\n\r\n",),
            headers=(("Transfer-Encoding", encoding),),
            expected_error="Unsupported Transfer-Encoding",
        )
        for name, encoding in (
            ("unsupported-encoding", "gzip, chunked"),
            ("unknown-encoding", "identity"),
        )
    }
)

HTTP11_HEADER_ERROR_RESPONSES = {
    "conflicting-framing": FetchHTTP11Response(
        (b"5\r\nhello\r\n0\r\n\r\n",),
        headers=(("Transfer-Encoding", "chunked"), ("Content-Length", "1")),
    ),
}
HTTP11_ERROR_RESPONSES.update(
    {
        name: FetchHTTP11Response(
            (packet,), expected_error="Inactivity timeout", keep_open=True
        )
        for name, packet in (
            ("stalled-size", b"1"),
            ("stalled-data", b"5\r\nhe"),
        )
    }
)


class FetchHTTPServer(http.server.ThreadingHTTPServer):
    """Threaded host server with request capture and deterministic stall release."""

    daemon_threads = True

    def __init__(self, server_address, handler_class):
        self.requests = queue.Queue()
        self.closed_requests = queue.Queue()
        self.release_stall = threading.Event()
        self.stalled_request_closed = threading.Event()
        super().__init__(server_address, handler_class)

    def server_bind(self):
        # HTTPServer.server_bind() calls getfqdn(), which stalls on the USB-net
        # address because the bench has no reverse DNS.
        socketserver.TCPServer.server_bind(self)
        self.server_name, self.server_port = self.server_address[:2]

    def url(self, path):
        host, port = self.server_address[:2]
        return f"http://{host}:{port}{path}"


class FetchRequestHandler(http.server.BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.0"

    def do_GET(self):
        self._handle_request()

    def do_POST(self):
        self._handle_request()

    def do_PUT(self):
        self._handle_request()

    def do_HEAD(self):
        self._handle_request()

    def _handle_request(self):
        parsed_url = urlsplit(self.path)
        request_path = parsed_url.path
        content_length = int(self.headers.get("Content-Length", "0"))
        body = self.rfile.read(content_length) if content_length else b""
        header_items = list(self.headers.raw_items())
        headers = {name.lower(): value for name, value in header_items}
        peer_certificate = None
        tls_version = None
        if isinstance(self.connection, ssl.SSLSocket):
            peer_certificate = self.connection.getpeercert(binary_form=True)
            tls_version = self.connection.version()
        self.server.requests.put(
            {
                "method": self.command,
                "version": self.request_version,
                "path": self.path,
                "body": body,
                "headers": headers,
                "header_items": header_items,
                "peer_certificate": peer_certificate,
                "tls_version": tls_version,
            }
        )

        if request_path.startswith("/http11/bodyless/"):
            _, _, _, status, framing, connection = request_path.split("/")
            headers = {
                "absent": (),
                "content-length": (("Content-Length", "123"),),
                "chunked": (("Transfer-Encoding", "chunked"),),
            }[framing]
            self._send_http11_response(
                FetchHTTP11Response(
                    (), headers=headers, keep_open=connection == "keep-alive"
                ),
                status=int(status),
            )
        elif request_path.startswith("/http11/"):
            name = request_path.removeprefix("/http11/")
            response = {
                **HTTP11_SUCCESS_RESPONSES,
                **HTTP11_ERROR_RESPONSES,
                **HTTP11_HEADER_ERROR_RESPONSES,
            }[name]
            self._send_http11_response(response)
        elif request_path == "/known.bin":
            self._send_payload(200, KNOWN_PAYLOAD)
        elif request_path == "/known-keep-alive.bin":
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Length", str(len(KNOWN_PAYLOAD)))
            self.send_header("Connection", "keep-alive")
            self.end_headers()
            self.wfile.write(KNOWN_PAYLOAD)
            self.wfile.flush()
            self.server.release_stall.wait(timeout=15)
        elif request_path == "/text":
            self._send_payload(200, TEXT_PAYLOAD, "text/plain")
        elif request_path == "/unicode":
            self._send_payload(200, UNICODE_TEXT_PAYLOAD, "text/plain; charset=utf-8")
        elif request_path == "/unicode-non-bmp":
            self._send_payload(
                200,
                NON_BMP_UNICODE_TEXT_PAYLOAD,
                "text/plain; charset=utf-8",
            )
        elif request_path == "/query":
            query = {
                name: values[-1]
                for name, values in parse_qs(
                    parsed_url.query, keep_blank_values=True
                ).items()
            }
            payload = json.dumps(query, ensure_ascii=False).encode("utf-8")
            self._send_payload(200, payload, "application/json; charset=utf-8")
        elif request_path == "/empty":
            self._send_payload(204, EMPTY_PAYLOAD)
        elif request_path == "/json":
            self._send_payload(200, JSON_PAYLOAD, "application/json")
        elif request_path == "/created":
            self._send_payload(201, CREATED_PAYLOAD, "application/json")
        elif request_path == "/server-error":
            self._send_payload(500, SERVER_ERROR_PAYLOAD, "application/json")
        elif request_path == "/invalid-json":
            self._send_payload(200, INVALID_JSON_PAYLOAD, "application/json")
        elif request_path == "/invalid-utf8":
            self._send_payload(200, INVALID_UTF8_PAYLOAD, "text/plain; charset=utf-8")
        elif request_path == "/headers":
            self._send_payload(
                200,
                HEADERS_PAYLOAD,
                headers={"X-Fetch-Alpha": "alpha", "X-Fetch-Number": "42"},
            )
        elif request_path in EMPTY_HEADER_RESPONSE_STATUSES:
            status = EMPTY_HEADER_RESPONSE_STATUSES[request_path]
            payload = EMPTY_PAYLOAD if status == 204 else HEADERS_PAYLOAD
            self._send_raw_headers(status, EMPTY_RESPONSE_HEADERS, payload)
        elif request_path in MALFORMED_RESPONSE_HEADERS:
            is_stalled = request_path == "/invalid-headers-stall"
            self._send_raw_headers(
                200,
                MALFORMED_RESPONSE_HEADERS[request_path],
                HEADERS_PAYLOAD,
                send_body=not is_stalled,
            )
            if is_stalled:
                self.connection.settimeout(HEADER_DISCONNECT_TIMEOUT_S)
                try:
                    if self.rfile.read(1) == b"":
                        self.server.stalled_request_closed.set()
                except TimeoutError:
                    pass
        elif request_path == "/truncated.bin":
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Length", str(len(TRUNCATED_PAYLOAD) * 2))
            self.send_header("Connection", "close")
            self.end_headers()
            self.wfile.write(TRUNCATED_PAYLOAD)
            self.wfile.flush()
            self.close_connection = True
        elif request_path == "/truncated":
            self.send_response(200)
            self.send_header("Content-Type", "text/plain")
            self.send_header("Content-Length", str(len(TRUNCATED_PAYLOAD) * 2))
            self.send_header("Connection", "close")
            self.end_headers()
            self.wfile.write(TRUNCATED_PAYLOAD)
            self.wfile.flush()
            self.close_connection = True
        elif request_path == "/request":
            self._send_payload(200, REQUEST_RESPONSE)
        elif request_path == "/not-found":
            self._send_payload(404, NOT_FOUND_PAYLOAD)
        elif request_path == "/unknown.bin":
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Connection", "close")
            self.end_headers()
            self.wfile.write(UNKNOWN_LENGTH_PAYLOAD)
            self.wfile.flush()
            self.close_connection = True
        elif request_path == "/stall":
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Length", "1")
            self.end_headers()
            self.wfile.flush()
            self.server.release_stall.wait(timeout=15)
        elif request_path == "/disconnect":
            self.close_connection = True
        else:
            self._send_payload(404, NOT_FOUND_PAYLOAD)

    def _send_http11_response(self, response, status=200):
        connection = "keep-alive" if response.keep_open else "close"
        headers = (
            f"HTTP/1.1 {status} {HTTPStatus(status).phrase}\r\n"
            "Content-Type: application/octet-stream\r\n"
            f"Connection: {connection}\r\n"
        )
        headers += "".join(f"{name}: {value}\r\n" for name, value in response.headers)
        try:
            self.wfile.write((headers + "\r\n").encode("ascii"))
            self.wfile.flush()
            for packet in response.packets:
                # Small deliberate gaps exercise streaming across separate TCP reads.
                if response.packet_interval_s:
                    self.server.release_stall.wait(response.packet_interval_s)
                self.wfile.write(packet)
                self.wfile.flush()
            if response.keep_open:
                self._wait_for_client_close()
        except (BrokenPipeError, ConnectionResetError):
            self.server.closed_requests.put(self.path)
        finally:
            self.close_connection = True

    def _wait_for_client_close(self):
        deadline = time.monotonic() + FETCH_RESPONSE_HOLD_S
        self.connection.settimeout(FETCH_CONNECTION_POLL_S)
        while not self.server.release_stall.is_set() and time.monotonic() < deadline:
            try:
                if not self.connection.recv(1):
                    self.server.closed_requests.put(self.path)
                    break
            except TimeoutError:
                continue

    def _send_raw_headers(self, status, raw_headers, payload, *, send_body=True):
        # These cases require exact bytes, including missing spaces and bare LF.
        response = (
            f"HTTP/1.1 {status} {HTTPStatus(status).phrase}\r\n"
            "Content-Type: text/plain\r\n"
            f"Content-Length: {len(payload)}\r\n"
            "Connection: close\r\n"
            "X-Before: before\r\n"
        ).encode("ascii")
        self.wfile.write(
            response + raw_headers + b"X-After: after\r\nX-Last-Empty:\r\n\r\n"
        )
        if send_body:
            self.wfile.write(payload)
        self.wfile.flush()
        self.close_connection = True

    def _send_payload(
        self,
        status,
        payload,
        content_type="application/octet-stream",
        headers=None,
    ):
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(payload)))
        for name, value in (headers or {}).items():
            self.send_header(name, value)
        self.end_headers()
        self.wfile.write(payload)

    def log_message(self, *args):
        pass
