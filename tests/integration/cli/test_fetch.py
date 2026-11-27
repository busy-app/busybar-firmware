"""`fetch` against a deterministic HTTP server on the pytest host."""

import hashlib
import queue

import allure
import pytest

from utils.fetch_http_server import (
    FETCH_HTTP11_TIMEOUT_S,
    HTTP11_ERROR_RESPONSES,
    HTTP11_HEADER_ERROR_RESPONSES,
    HTTP11_INCOMPLETE_TRAILER_CASES,
    HTTP11_INFORMATIONAL_RESPONSES,
    HTTP11_SUCCESS_RESPONSES,
    KNOWN_PAYLOAD,
    NOT_FOUND_PAYLOAD,
    REQUEST_RESPONSE,
    UNKNOWN_LENGTH_PAYLOAD,
)

pytestmark = [
    pytest.mark.cli,
    pytest.mark.bsb_metadata(
        suite="Integration",
        level="Integration",
        intent="Integration",
        interfaces=("CLI",),
        transports=("Telnet",),
    ),
]

FETCH_USAGE = """Usage:
fetch [options] <url>
Options:
-o Output file path
-d HTTP POST/PUT data
-H Custom header(s)
-X Request method
-k (TLS) Ignore server certificate
-a (TLS) Client auth type ("none" (default), "device" or "cert")
-C (TLS) Custom client certificate file path
-K (TLS) Custom client private key file path
-v Enable full output"""

HTTP11_FAILED_RESPONSES = {**HTTP11_ERROR_RESPONSES, **HTTP11_HEADER_ERROR_RESPONSES}


@allure.epic("Device Connectivity")
@allure.feature("Outbound HTTP/TLS")
@allure.story("HTTP request and download lifecycle")
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

    @staticmethod
    def _assert_http11_request(http_server, path, output, method="GET"):
        with allure.step("Verify the captured HTTP/1.1 request"):
            try:
                captured = http_server.requests.get(timeout=1)
            except queue.Empty:
                pytest.fail(f"No request for {path!r}; Fetch output={output!r}")
            assert captured["version"] == "HTTP/1.1", f"captured={captured!r}"
            assert captured["path"] == path, f"captured={captured!r}"
            assert captured["method"] == method, f"captured={captured!r}"

    @allure.title("CLI. Command fetch requires a URL.")
    @pytest.mark.regression
    def test_fetch_requires_url(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("fetch")

        with allure.step("Verify missing-URL error and usage"):
            assert "Error: No URL specified" in response, response
            assert "fetch [options] <url>" in response, response

    @allure.title("CLI. Command fetch prints the complete usage contract.")
    @pytest.mark.regression
    def test_fetch_usage_contract(self, persistent_cli_connection):
        response = persistent_cli_connection.execute_command("fetch")

        with allure.step("Extract and verify the complete Fetch usage block"):
            prefix, separator, usage = response.partition("Usage:")
            assert separator, f"Fetch usage header is missing: {response!r}"
            assert prefix.strip() == "Error: No URL specified", response
            assert (
                f"Usage:{usage}" == FETCH_USAGE
            ), f"Fetch usage differs from its public CLI contract: {response!r}"

    @allure.title("CLI. Command fetch rejects invalid arguments.")
    @pytest.mark.parametrize(
        "command",
        ["fetch -Z", "fetch -o"],
        ids=["unknown-option", "missing-option-value"],
    )
    @pytest.mark.regression
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
    @pytest.mark.regression
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
    @pytest.mark.regression
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
            assert captured["version"] == "HTTP/1.1", f"captured={captured!r}"
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
                assert "HTTP/1.0 200 OK" in response, (
                    f"expected close-delimited success for {captured!r}, "
                    f"got {response!r}"
                )
                assert "Content-Length" not in response, response
                assert "Transfer-Encoding" not in response, response

            self._assert_saved_payload(cli, self.UNKNOWN_DEST, UNKNOWN_LENGTH_PAYLOAD)
        finally:
            cli.execute_command(f"storage remove {self.UNKNOWN_DEST}")

    @allure.title("CLI. Command fetch accepts a scheme-less explicit-port URL.")
    @pytest.mark.regression
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
    @pytest.mark.regression
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

    @allure.title("CLI. Fetch saves decoded HTTP/1.1 data: {case_name}.")
    @pytest.mark.parametrize(
        "case_name",
        [
            pytest.param(
                case_name,
                marks=(
                    pytest.mark.regression
                    if case_name not in {"simple", "trailers"}
                    else ()
                ),
            )
            for case_name in HTTP11_SUCCESS_RESPONSES
        ],
    )
    def test_fetch_http11_download(
        self, persistent_cli_connection, http_server, storage_dir, case_name
    ):
        cli = persistent_cli_connection
        case = HTTP11_SUCCESS_RESPONSES[case_name]
        path = f"/http11/{case_name}"
        destination = f"{storage_dir}/http11.bin"
        response = cli.execute_command(
            f"fetch -v {http_server.url(path)} -o {destination}",
            timeout=FETCH_HTTP11_TIMEOUT_S,
            slow_command=True,
        )
        with allure.step("Verify successful completion and decoded progress"):
            assert "HTTP/1.1 200 OK" in response, response
            assert "Error:" not in response, response
            if any(name.lower() == "transfer-encoding" for name, _ in case.headers):
                expected_progress = f"{len(case.payload) // 1024}KiB/?KiB"
                assert (
                    expected_progress in response
                ), f"Expected decoded progress {expected_progress!r}, got {response!r}"
            elif case.payload and case.keep_open:
                assert "Downloaded: 100%" in response, response

        self._assert_http11_request(http_server, path, response)
        self._assert_saved_payload(cli, destination, case.payload)

    @allure.title("CLI. Fetch rejects broken chunked data and recovers: {case_name}.")
    @pytest.mark.parametrize(
        "case_name",
        [
            (
                pytest.param(
                    case_name,
                    marks=(pytest.mark.regression, pytest.mark.skip(reason="FW-1313")),
                )
                if case_name in HTTP11_INCOMPLETE_TRAILER_CASES
                else pytest.param(
                    case_name,
                    marks=(
                        pytest.mark.regression
                        if case_name not in {"conflicting-framing", "stalled-size"}
                        else ()
                    ),
                )
            )
            for case_name in HTTP11_FAILED_RESPONSES
        ],
    )
    def test_fetch_http11_error_removes_output(
        self, persistent_cli_connection, http_server, storage_dir, case_name
    ):
        cli = persistent_cli_connection
        case = HTTP11_FAILED_RESPONSES[case_name]
        path = f"/http11/{case_name}"
        destination = f"{storage_dir}/http11.bin"
        response = cli.execute_command(
            f"fetch {http_server.url(path)} -o {destination}",
            timeout=FETCH_HTTP11_TIMEOUT_S,
            slow_command=True,
        )
        with allure.step("Verify the framing or inactivity error"):
            expected_error = (
                f"Error: {case.expected_error}" if case.expected_error else "Error:"
            )
            assert expected_error in response, response
        self._assert_http11_request(http_server, path, response)
        with allure.step("Verify the partial file was removed"):
            stat = cli.execute_command(f"storage stat {destination}")
            assert (
                "Storage error: file/dir not exist" in stat
            ), f"Partial file remains: {stat!r}; Fetch output={response!r}"
        with allure.step("Download again after the failed transfer"):
            recovery = cli.execute_command(
                f"fetch {http_server.url('/http11/simple')} -o {destination}",
                timeout=FETCH_HTTP11_TIMEOUT_S,
                slow_command=True,
            )
            assert "Error:" not in recovery, recovery
        self._assert_http11_request(http_server, "/http11/simple", recovery)
        self._assert_saved_payload(cli, destination, b"hello")

    @allure.title("CLI. Fetch rejects a 1xx response: {case_name}, {method}.")
    @pytest.mark.parametrize("case_name", HTTP11_INFORMATIONAL_RESPONSES)
    @pytest.mark.parametrize("method", ["GET", "HEAD"])
    @pytest.mark.regression
    def test_fetch_http11_informational_response(
        self, persistent_cli_connection, http_server, storage_dir, case_name, method
    ):
        cli = persistent_cli_connection
        case = HTTP11_INFORMATIONAL_RESPONSES[case_name]
        path = f"/http11/{case_name}"
        destination = f"{storage_dir}/informational.bin"
        response = cli.execute_command(
            f"fetch -X {method} {http_server.url(path)} -o {destination}",
            timeout=FETCH_HTTP11_TIMEOUT_S,
            slow_command=True,
        )
        with allure.step("Verify the informational status was explicitly rejected"):
            assert f"Error: {case.expected_error}" in response, response
        self._assert_http11_request(http_server, path, response, method)
        with allure.step("Verify the rejected download left no output file"):
            stat = cli.execute_command(f"storage stat {destination}")
            assert (
                "Storage error: file/dir not exist" in stat
            ), f"Rejected file remains: {stat!r}; Fetch output={response!r}"
        if case.keep_open:
            with allure.step("Verify Fetch closed the keep-alive connection"):
                try:
                    closed_path = http_server.closed_requests.get(timeout=1)
                except queue.Empty:
                    pytest.fail(
                        f"Rejected connection remains open; output={response!r}"
                    )
                assert closed_path == path, f"Closed {closed_path!r}, expected {path!r}"
        with allure.step("Verify a subsequent HTTP 200 download succeeds"):
            recovery = cli.execute_command(
                f"fetch {http_server.url('/http11/simple')} -o {destination}",
                timeout=FETCH_HTTP11_TIMEOUT_S,
                slow_command=True,
            )
            assert "Error:" not in recovery, recovery
        self._assert_http11_request(http_server, "/http11/simple", recovery)
        self._assert_saved_payload(cli, destination, b"hello")

    @allure.title("CLI. Fetch completes a bodyless HTTP/1.1 response.")
    @pytest.mark.parametrize(
        "method,status",
        [("HEAD", 200), ("GET", 204), ("GET", 304)],
        ids=["head", "no-content", "not-modified"],
    )
    @pytest.mark.parametrize("framing", ["absent", "content-length", "chunked"])
    @pytest.mark.parametrize("connection", ["close", "keep-alive"])
    @pytest.mark.regression
    def test_fetch_http11_bodyless_response(
        self,
        persistent_cli_connection,
        http_server,
        storage_dir,
        method,
        status,
        framing,
        connection,
    ):
        cli = persistent_cli_connection
        path = f"/http11/bodyless/{status}/{framing}/{connection}"
        destination = f"{storage_dir}/bodyless.bin"
        response = cli.execute_command(
            f"fetch -v -X {method} {http_server.url(path)} -o {destination}",
            timeout=FETCH_HTTP11_TIMEOUT_S,
            slow_command=True,
        )
        with allure.step("Verify headers-only completion without a timeout"):
            assert f"HTTP/1.1 {status} " in response, response
            assert "Error:" not in response, response
            assert "?KiB" not in response, response
        self._assert_http11_request(http_server, path, response, method)
        self._assert_saved_payload(cli, destination, b"")
