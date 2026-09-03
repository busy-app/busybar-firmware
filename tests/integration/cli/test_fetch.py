"""`fetch` against a deterministic HTTP server on the pytest host."""

import hashlib
import queue

import allure
import pytest

from utils.fetch_http_server import (
    KNOWN_PAYLOAD,
    NOT_FOUND_PAYLOAD,
    REQUEST_RESPONSE,
    UNKNOWN_LENGTH_PAYLOAD,
)

pytestmark = pytest.mark.cli


@allure.epic("BSB CLI Testing")
@allure.feature("6. CLI")
@allure.story("Commands Check")
class TestCLIFetch:
    """Exercise the real device Fetch path through a local host server."""

    DEST = "/ext/fetch_test.bin"
    KEEP_ALIVE_DEST = "/ext/fetch_keep_alive.bin"
    TRUNCATED_DEST = "/ext/fetch_truncated.bin"
    UNKNOWN_DEST = "/ext/fetch_unknown_length.bin"
    TIMEOUT_DEST = "/ext/fetch_timeout.bin"

    @staticmethod
    def _assert_saved_payload(cli, path, payload):
        with allure.step(f"Verify exact saved payload at {path}"):
            stat = cli.execute_command(f"storage stat {path}")
            assert (
                f"size: {len(payload)}b" in stat
            ), f"expected {len(payload)} bytes at {path}, got stat output {stat!r}"

            expected_md5 = hashlib.md5(payload).hexdigest()
            actual_md5 = cli.execute_command(f"storage md5 {path}")
            assert (
                expected_md5 in actual_md5
            ), f"expected MD5 {expected_md5} for {path}, got {actual_md5!r}"

    @allure.title("CLI. Command fetch requires a URL.")
    def test_fetch_requires_url(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("fetch")

        with allure.step("Verify missing-URL error and usage"):
            assert "Error: No URL specified" in response, response
            assert "fetch [options] <url>" in response, response

    @allure.title("CLI. Command fetch rejects invalid arguments.")
    @pytest.mark.parametrize(
        "command",
        ["fetch -Z", "fetch -o"],
        ids=["unknown-option", "missing-option-value"],
    )
    def test_fetch_rejects_invalid_arguments(self, persistent_cli_connection, command):
        response = persistent_cli_connection.execute_command(command)

        with allure.step("Verify argument error and usage"):
            assert (
                "Error: Invalid arguments" in response
            ), f"expected invalid-arguments error for {command!r}, got {response!r}"
            assert (
                "fetch [options] <url>" in response
            ), f"expected Fetch usage for {command!r}, got {response!r}"

    @allure.title("CLI. Command fetch streams verbose response to stdout.")
    def test_fetch_to_stdout(self, persistent_cli_connection, http_server):
        response = persistent_cli_connection.execute_command(
            f"fetch -v {http_server.url('/known.bin')}",
            timeout=25,
            slow_command=True,
        )

        with allure.step("Verify response headers and payload marker"):
            assert "HTTP/1.0 200 OK" in response, response
            assert "busybar-fetch-test" in response, response

    @allure.title("CLI. Command fetch writes an exact known-length file.")
    def test_fetch_to_file(self, persistent_cli_connection, http_server):
        cli = persistent_cli_connection
        cli.execute_command(f"storage remove {self.DEST}")
        try:
            response = cli.execute_command(
                f"fetch {http_server.url('/known.bin')} -o {self.DEST}",
                timeout=25,
                slow_command=True,
            )
            with allure.step("Verify known-length completion output"):
                assert "Downloaded: 100%" in response, response

            self._assert_saved_payload(cli, self.DEST, KNOWN_PAYLOAD)
        finally:
            cli.execute_command(f"storage remove {self.DEST}")

    @allure.title(
        "CLI. Fetch completes a known-length response before the connection closes."
    )
    def test_fetch_known_length_does_not_wait_for_connection_close(
        self, persistent_cli_connection, http_server
    ):
        cli = persistent_cli_connection
        cli.execute_command(f"storage remove {self.KEEP_ALIVE_DEST}")
        try:
            response = cli.execute_command(
                f"fetch {http_server.url('/known-keep-alive.bin')} "
                f"-o {self.KEEP_ALIVE_DEST}",
                timeout=12,
                slow_command=True,
            )

            with allure.step("Verify completion without an inactivity failure"):
                assert "Downloaded: 100%" in response, response
                assert "Inactivity timeout" not in response, response

            self._assert_saved_payload(cli, self.KEEP_ALIVE_DEST, KNOWN_PAYLOAD)
        finally:
            http_server.release_stall.set()
            cli.execute_command(f"storage remove {self.KEEP_ALIVE_DEST}")

    @allure.title("CLI. Command fetch rejects a truncated known-length response.")
    def test_fetch_truncated_content_length_removes_output(
        self, persistent_cli_connection, http_server
    ):
        cli = persistent_cli_connection
        cli.execute_command(f"storage remove {self.TRUNCATED_DEST}")
        try:
            response = cli.execute_command(
                f"fetch {http_server.url('/truncated.bin')} -o {self.TRUNCATED_DEST}",
                timeout=25,
                slow_command=True,
            )
            with allure.step("Verify truncated response failure"):
                assert "Error: Incomplete response body" in response, response

            with allure.step("Verify incomplete destination was removed"):
                stat = cli.execute_command(f"storage stat {self.TRUNCATED_DEST}")
                assert "Storage error: file/dir not exist" in stat, (
                    f"expected no file at {self.TRUNCATED_DEST}, got {stat!r}; "
                    f"Fetch output was {response!r}"
                )
        finally:
            cli.execute_command(f"storage remove {self.TRUNCATED_DEST}")

    @allure.title("CLI. Command fetch preserves method, body, and custom header.")
    def test_fetch_request_semantics(self, persistent_cli_connection, http_server):
        body = "body value with spaces"
        header_value = "header value with spaces"
        command = (
            f'fetch -X PUT -d "{body}" {http_server.url("/request")} '
            f'-H "X-Fetch-Test: {header_value}"'
        )
        response = persistent_cli_connection.execute_command(
            command, timeout=25, slow_command=True
        )

        with allure.step("Read the request captured by the host server"):
            try:
                captured = http_server.requests.get(timeout=1)
            except queue.Empty:
                pytest.fail(
                    f"host server captured no request; Fetch output was {response!r}"
                )

        with allure.step("Verify method, quoted values, and request framing"):
            assert (
                captured["method"] == "PUT"
            ), f"expected method PUT, captured {captured!r}"
            assert (
                captured["body"] == body.encode()
            ), f"expected body {body.encode()!r}, captured {captured!r}"
            assert (
                captured["headers"].get("x-fetch-test") == header_value
            ), f"expected custom header {header_value!r}, captured {captured!r}"
            assert captured["headers"].get("content-length") == str(
                len(body.encode())
            ), f"expected Content-Length {len(body.encode())}, captured {captured!r}"
            assert (
                captured["headers"].get("transfer-encoding") != "chunked"
            ), f"request must not be chunked, captured {captured!r}"
            assert REQUEST_RESPONSE.decode().strip() in response, response

    @allure.title("CLI. Command fetch exposes a 404 status and response body.")
    def test_fetch_http_error_passthrough(self, persistent_cli_connection, http_server):
        response = persistent_cli_connection.execute_command(
            f"fetch {http_server.url('/not-found')} -v",
            timeout=25,
            slow_command=True,
        )

        with allure.step("Verify HTTP error passthrough"):
            assert "HTTP/1.0 404 Not Found" in response, response
            assert NOT_FOUND_PAYLOAD.decode().strip() in response, response

    @allure.title("CLI. Command fetch saves a close-delimited response.")
    def test_fetch_without_content_length(self, persistent_cli_connection, http_server):
        cli = persistent_cli_connection
        cli.execute_command(f"storage remove {self.UNKNOWN_DEST}")
        try:
            response = cli.execute_command(
                f"fetch -v {http_server.url('/unknown.bin')} -o {self.UNKNOWN_DEST}",
                timeout=25,
                slow_command=True,
            )
            try:
                captured = http_server.requests.get(timeout=1)
            except queue.Empty:
                pytest.fail(
                    f"host server captured no request; Fetch output was {response!r}"
                )

            with allure.step("Verify close-delimited response framing"):
                assert captured["path"] == "/unknown.bin", (
                    f"expected request path '/unknown.bin', captured {captured!r}; "
                    f"Fetch output was {response!r}"
                )
                assert (
                    "HTTP/1.0 200 OK" in response
                ), f"expected close-delimited success for {captured!r}, got {response!r}"
                assert "Content-Length" not in response, response
                assert "Transfer-Encoding" not in response, response

            self._assert_saved_payload(cli, self.UNKNOWN_DEST, UNKNOWN_LENGTH_PAYLOAD)
        finally:
            cli.execute_command(f"storage remove {self.UNKNOWN_DEST}")

    @allure.title("CLI. Command fetch accepts a scheme-less explicit-port URL.")
    def test_fetch_scheme_less_url(self, persistent_cli_connection, http_server):
        scheme_less_url = http_server.url("/known.bin").removeprefix("http://")
        response = persistent_cli_connection.execute_command(
            f"fetch -v {scheme_less_url}",
            timeout=25,
            slow_command=True,
        )
        try:
            captured = http_server.requests.get(timeout=1)
        except queue.Empty:
            pytest.fail(
                f"host server captured no request for {scheme_less_url!r}; "
                f"Fetch output was {response!r}"
            )

        with allure.step("Verify scheme-less URL request path and response"):
            assert captured["path"] == "/known.bin", (
                f"expected request path '/known.bin' for {scheme_less_url!r}, "
                f"captured {captured!r}; Fetch output was {response!r}"
            )
            assert "HTTP/1.0 200 OK" in response, (
                f"expected scheme-less URL success, captured {captured!r}; "
                f"Fetch output was {response!r}"
            )
            assert KNOWN_PAYLOAD.decode().strip() in response, (
                f"expected payload {KNOWN_PAYLOAD!r}, captured {captured!r}; "
                f"Fetch output was {response!r}"
            )

    @allure.title("CLI. Command fetch removes output after inactivity timeout.")
    def test_fetch_inactivity_removes_output(
        self, persistent_cli_connection, http_server
    ):
        cli = persistent_cli_connection
        cli.execute_command(f"storage remove {self.TIMEOUT_DEST}")
        try:
            response = cli.execute_command(
                f"fetch {http_server.url('/stall')} -o {self.TIMEOUT_DEST}",
                timeout=12,
                slow_command=True,
            )
            with allure.step("Verify deterministic inactivity failure"):
                assert "Inactivity timeout" in response, response

            with allure.step("Verify failed destination was removed"):
                stat = cli.execute_command(f"storage stat {self.TIMEOUT_DEST}")
                assert (
                    "Storage error: file/dir not exist" in stat
                ), f"expected no file at {self.TIMEOUT_DEST}, got {stat!r}"
        finally:
            http_server.release_stall.set()
            cli.execute_command(f"storage remove {self.TIMEOUT_DEST}")
