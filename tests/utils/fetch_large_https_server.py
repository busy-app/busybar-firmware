"""Bounded binary HTTPS responses for Wi-Fi download checks."""

import http.server


LARGE_HTTPS_PAYLOAD = bytes(range(256)) * 256
LARGE_HTTPS_CHUNK_SIZE = 4096


class LargeHTTPSRequestHandler(http.server.BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def do_GET(self):
        framing = self.path.removeprefix("/large/")
        if self.path != f"/large/{framing}" or framing not in {
            "length",
            "chunked",
            "close",
        }:
            self.send_error(404)
            return

        self.server.requests.put(
            {
                "peer_ip": self.client_address[0],
                "tls_version": self.connection.version(),
                "framing": framing,
            }
        )
        self.send_response(200)
        self.send_header("Content-Type", "application/octet-stream")
        self.send_header("Connection", "close")
        if framing == "length":
            self.send_header("Content-Length", str(len(LARGE_HTTPS_PAYLOAD)))
        elif framing == "chunked":
            self.send_header("Transfer-Encoding", "chunked")
        self.end_headers()

        if framing == "chunked":
            for offset in range(0, len(LARGE_HTTPS_PAYLOAD), LARGE_HTTPS_CHUNK_SIZE):
                chunk = LARGE_HTTPS_PAYLOAD[offset : offset + LARGE_HTTPS_CHUNK_SIZE]
                self.wfile.write(f"{len(chunk):x}\r\n".encode("ascii"))
                self.wfile.write(chunk + b"\r\n")
            self.wfile.write(b"0\r\n\r\n")
        else:
            self.wfile.write(LARGE_HTTPS_PAYLOAD)
        self.wfile.flush()
        self.close_connection = True

    def log_message(self, format, *args):
        pass
