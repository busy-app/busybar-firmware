"""`fetch` against an HTTP server served from the test host itself.

Coverage matrix and plan: scratchpad/cli_coverage_matrix.md.
"""

import hashlib
import http.server
import socketserver
import threading

import allure
import pytest

pytestmark = pytest.mark.cli


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLIFetch:
    """`fetch` against an HTTP server served from the test host itself.

    The device reaches the host over the same link the CLI runs on, so bind the
    server to the local end of the telnet socket — no bench configuration needed.
    """

    PAYLOAD = b"busybar-fetch-test\n" * 10
    DEST = "/ext/fetch_test.bin"

    @pytest.fixture
    def http_server(self, persistent_cli_connection):
        payload = self.PAYLOAD

        class Handler(http.server.BaseHTTPRequestHandler):
            def do_GET(self):
                self.send_response(200)
                self.send_header("Content-Type", "application/octet-stream")
                self.send_header("Content-Length", str(len(payload)))
                self.end_headers()
                self.wfile.write(payload)

            def log_message(self, *args):
                pass

        class Server(http.server.ThreadingHTTPServer):
            # HTTPServer.server_bind() resolves the bound address with getfqdn(),
            # which stalls ~5s on the bench (no reverse DNS for the USB-net range)
            def server_bind(self):
                socketserver.TCPServer.server_bind(self)
                self.server_name, self.server_port = self.server_address[:2]

        host_ip = persistent_cli_connection.tn.sock.getsockname()[0]
        server = Server((host_ip, 0), Handler)
        threading.Thread(target=server.serve_forever, daemon=True).start()
        try:
            yield f"http://{host_ip}:{server.server_address[1]}/payload.bin"
        finally:
            server.shutdown()
            server.server_close()

    @allure.title("CLI. Command fetch (usage).")
    def test_fetch_usage(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("fetch")
        assert "fetch [options] <url>" in response, response

    @allure.title("CLI. Command fetch (download to stdout).")
    def test_fetch_to_stdout(self, persistent_cli_connection, http_server):
        response = persistent_cli_connection.execute_command(
            f"fetch -v {http_server}", timeout=25, slow_command=True
        )
        assert "HTTP/1.0 200 OK" in response, response
        assert "busybar-fetch-test" in response, response

    @allure.title("CLI. Command fetch (download to file).")
    def test_fetch_to_file(self, persistent_cli_connection, http_server):
        cli = persistent_cli_connection
        cli.execute_command(f"storage remove {self.DEST}")
        try:
            response = cli.execute_command(
                f"fetch {http_server} -o {self.DEST}", timeout=25, slow_command=True
            )
            assert "Downloaded: 100%" in response, response

            stat = cli.execute_command(f"storage stat {self.DEST}")
            assert f"size: {len(self.PAYLOAD)}b" in stat, stat
            md5 = cli.execute_command(f"storage md5 {self.DEST}")
            assert hashlib.md5(self.PAYLOAD).hexdigest() in md5, (
                f"downloaded file differs from what was served: {md5!r}"
            )
        finally:
            cli.execute_command(f"storage remove {self.DEST}")
