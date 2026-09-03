"""Deterministic HTTP server shared by device-side Fetch integration tests."""

import http.server
import json
import queue
import socketserver
import threading
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
HEADERS_PAYLOAD = b"headers-ok\n"
CREATED_PAYLOAD = b'{"created":true}'
SERVER_ERROR_PAYLOAD = b'{"error":"server-error"}'
TRUNCATED_PAYLOAD = b"truncated-fetch-test\n" * 3
UNKNOWN_LENGTH_PAYLOAD = b"close-delimited-fetch-payload\n" * 7
NOT_FOUND_PAYLOAD = b"fetch-route-not-found\n"
REQUEST_RESPONSE = b"request-captured\n"


class FetchHTTPServer(http.server.ThreadingHTTPServer):
    """Threaded host server with request capture and deterministic stall release."""

    daemon_threads = True

    def __init__(self, server_address, handler_class):
        self.requests = queue.Queue()
        self.release_stall = threading.Event()
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

    def _handle_request(self):
        parsed_url = urlsplit(self.path)
        request_path = parsed_url.path
        content_length = int(self.headers.get("Content-Length", "0"))
        body = self.rfile.read(content_length) if content_length else b""
        headers = {name.lower(): value for name, value in self.headers.items()}
        self.server.requests.put(
            {
                "method": self.command,
                "path": self.path,
                "body": body,
                "headers": headers,
            }
        )

        if request_path == "/known.bin":
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
        elif request_path == "/headers":
            self._send_payload(
                200,
                HEADERS_PAYLOAD,
                headers={"X-Fetch-Alpha": "alpha", "X-Fetch-Number": "42"},
            )
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
