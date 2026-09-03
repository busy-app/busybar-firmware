"""JavaScript Fetch API checks for JS Apps."""

import json
import queue
from textwrap import dedent

import allure
import pytest

from utils.fetch_http_server import (
    HEADERS_PAYLOAD,
    NON_BMP_UNICODE_TEXT,
    NOT_FOUND_PAYLOAD,
    REQUEST_RESPONSE,
    TEXT_PAYLOAD,
    UNICODE_TEXT,
)


pytestmark = pytest.mark.cli

UPDATE_DIRECTORY_URL = "https://update.busy.app/busybar-firmware/directory.json"


def captured_request(http_server, output, description):
    """Return the request observed by the host or fail with useful context."""
    try:
        return http_server.requests.get(timeout=1)
    except queue.Empty:
        pytest.fail(f"host server captured no {description}; output={output!r}")


@allure.epic("BSB CLI Testing")
@allure.feature("JavaScript Fetch API")
@allure.story("JS Apps")
class TestJSFetch:
    """Exercise JS Apps fetch semantics through the real device network."""

    @allure.title("JavaScript fetch performs a GET without a request body.")
    def test_get_text_response(self, js_case_runner, http_server):
        case_name = "get_text_response"
        url = json.dumps(http_server.url("/text"))
        expected_text = json.dumps(TEXT_PAYLOAD.decode("utf-8"))
        body = dedent(
            f"""
                const response = await fetch({url});
                assert(response.status === 200, "status=" + response.status);
                assert(response.statusText === "OK",
                    "statusText=" + response.statusText);
                assert(response.ok === true, "ok=" + response.ok);
                assert(response.type === "basic", "type=" + response.type);
                assert(response.bodyUsed === false, "bodyUsed before read");
                assert(response.headers.has("Content-Type"), "Content-Type missing");
                const text = await response.text();
                assert(text === {expected_text}, "unexpected response text");
                assert(response.bodyUsed === true, "bodyUsed after read");
            """
        ).strip()

        output = js_case_runner(case_name, body)

        with allure.step("Verify the host received a GET without a body"):
            captured = captured_request(http_server, output, "GET request")
            assert captured["method"] == "GET", f"captured={captured!r}"
            assert captured["path"] == "/text", f"captured={captured!r}"
            assert captured["body"] == b"", f"captured={captured!r}"

    @allure.title("JavaScript fetch preserves encoded query parameters.")
    def test_get_query_parameters(self, js_case_runner, http_server):
        case_name = "get_query_parameters"
        request_path = (
            "/query?plain=busybar&space=hello%20world&symbols=%26%3D"
            "&unicode=%D0%BF%D1%80%D0%B8%D0%B2%D0%B5%D1%82"
        )
        url = json.dumps(http_server.url(request_path))
        expected_unicode = json.dumps("привет")
        body = dedent(
            f"""
                const response = await fetch({url});
                assert(response.status === 200, "status=" + response.status);
                const data = await response.json();
                assert(data.plain === "busybar", "plain=" + data.plain);
                assert(data.space === "hello world", "space=" + data.space);
                assert(data.symbols === "&=", "symbols=" + data.symbols);
                assert(data.unicode === {expected_unicode},
                    "unicode=" + data.unicode);
            """
        ).strip()

        output = js_case_runner(case_name, body)

        with allure.step("Verify the encoded query reached the host unchanged"):
            captured = captured_request(http_server, output, "query request")
            assert captured["method"] == "GET", f"captured={captured!r}"
            assert captured["path"] == request_path, f"captured={captured!r}"

    @allure.title("JavaScript fetch decodes a Unicode text response.")
    def test_get_unicode_text_response(self, js_case_runner, http_server):
        case_name = "get_unicode_text_response"
        url = json.dumps(http_server.url("/unicode"))
        expected_text = json.dumps(UNICODE_TEXT)
        body = dedent(
            f"""
                const response = await fetch({url});
                assert(response.status === 200, "status=" + response.status);
                assert(await response.text() === {expected_text},
                    "unexpected Unicode response");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript fetch decodes a non-BMP Unicode text response.")
    def test_get_non_bmp_unicode_text_response(self, js_case_runner, http_server):
        case_name = "get_non_bmp_unicode_text_response"
        url = json.dumps(http_server.url("/unicode-non-bmp"))
        expected_text = json.dumps(NON_BMP_UNICODE_TEXT)
        body = dedent(
            f"""
                const response = await fetch({url});
                assert(response.status === 200, "status=" + response.status);
                assert(await response.text() === {expected_text},
                    "unexpected non-BMP Unicode response");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript Request constructor requires new.")
    def test_request_constructor_requires_new(self, js_case_runner):
        case_name = "request_requires_new"
        body = dedent(
            """
                let rejected = false;
                try {
                    Request("http://unused.invalid/");
                } catch (error) {
                    rejected = true;
                }
                assert(rejected, "Request() without new did not throw");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript fetch init overrides a Request object.")
    def test_request_init_overrides_request(self, js_case_runner, http_server):
        case_name = "request_init_override"
        url = json.dumps(http_server.url("/request"))
        payload = {"source": "override", "value": 7}
        body = dedent(
            f"""
                const request = new Request({url}, {{
                    method: "GET",
                    headers: {{"X-JS-Original": "original"}}
                }});
                const response = await fetch(request, {{
                    method: "POST",
                    headers: {{
                        "Content-Type": "application/json",
                        "X-JS-Override": "override"
                    }},
                    body: JSON.stringify({json.dumps(payload)})
                }});
                assert(response.status === 200, "status=" + response.status);
                await response.text();
            """
        ).strip()

        output = js_case_runner(case_name, body)

        with allure.step("Verify fetch init replaced the Request values"):
            captured = captured_request(http_server, output, "override request")
            assert captured["method"] == "POST", f"captured={captured!r}"
            assert json.loads(captured["body"]) == payload, f"captured={captured!r}"
            assert (
                captured["headers"].get("content-type") == "application/json"
            ), f"captured={captured!r}"
            assert (
                captured["headers"].get("x-js-override") == "override"
            ), f"captured={captured!r}"
            assert "x-js-original" not in captured["headers"], f"captured={captured!r}"

    @allure.title("JavaScript fetch performs a POST without a body.")
    def test_post_without_body(self, js_case_runner, http_server):
        case_name = "post_without_body"
        url = json.dumps(http_server.url("/request"))
        expected_response = json.dumps(REQUEST_RESPONSE.decode("utf-8"))
        body = dedent(
            f"""
                const response = await fetch({url}, {{method: "POST"}});
                assert(response.status === 200, "status=" + response.status);
                assert(await response.text() === {expected_response}, "response body");
            """
        ).strip()

        output = js_case_runner(case_name, body)

        with allure.step("Verify the host received an empty POST"):
            captured = captured_request(http_server, output, "empty POST request")
            assert captured["method"] == "POST", f"captured={captured!r}"
            assert captured["path"] == "/request", f"captured={captured!r}"
            assert captured["body"] == b"", f"captured={captured!r}"

    @allure.title("JavaScript fetch sends a JSON POST body and headers.")
    def test_post_json_request(self, js_case_runner, http_server):
        case_name = "post_json_request"
        url = json.dumps(http_server.url("/request"))
        payload = {
            "message": "Привет из JS App",
            "city": "Москва",
            "emoji": "👋",
            "value": 42,
        }
        expected_response = json.dumps(REQUEST_RESPONSE.decode("utf-8"))
        body = dedent(
            f"""
                const payload = {json.dumps(payload)};
                const response = await fetch({url}, {{
                    method: "POST",
                    headers: {{
                        "Content-Type": "application/json",
                        "X-JS-Fetch-Test": "json-post"
                    }},
                    body: JSON.stringify(payload)
                }});
                assert(response.status === 200, "status=" + response.status);
                assert(await response.text() === {expected_response}, "response body");
            """
        ).strip()

        output = js_case_runner(case_name, body)

        with allure.step("Verify the JSON POST captured by the host"):
            captured = captured_request(http_server, output, "JSON POST request")
            assert captured["method"] == "POST", f"captured={captured!r}"
            assert captured["path"] == "/request", f"captured={captured!r}"
            assert json.loads(captured["body"]) == payload, f"captured={captured!r}"
            assert (
                captured["headers"].get("content-type") == "application/json"
            ), f"captured={captured!r}"
            assert (
                captured["headers"].get("x-js-fetch-test") == "json-post"
            ), f"captured={captured!r}"

    @allure.title("JavaScript fetch sends a plain-text POST body.")
    def test_post_text_request(self, js_case_runner, http_server):
        case_name = "post_text_request"
        url = json.dumps(http_server.url("/request"))
        request_body = "plain text from JS App"
        expected_response = json.dumps(REQUEST_RESPONSE.decode("utf-8"))
        body = dedent(
            f"""
                const response = await fetch({url}, {{
                    method: "POST",
                    headers: {{"Content-Type": "text/plain"}},
                    body: {json.dumps(request_body)}
                }});
                assert(response.status === 200, "status=" + response.status);
                assert(await response.text() === {expected_response}, "response body");
            """
        ).strip()

        output = js_case_runner(case_name, body)

        with allure.step("Verify the plain-text POST captured by the host"):
            captured = captured_request(http_server, output, "plain-text POST request")
            assert captured["method"] == "POST", f"captured={captured!r}"
            assert captured["body"] == request_body.encode(), f"captured={captured!r}"
            assert (
                captured["headers"].get("content-type") == "text/plain"
            ), f"captured={captured!r}"

    @allure.title("JavaScript fetch parses a JSON response.")
    def test_get_json_response(self, js_case_runner, http_server):
        case_name = "get_json_response"
        url = json.dumps(http_server.url("/json"))
        body = dedent(
            f"""
                const response = await fetch({url});
                assert(response.status === 200, "status=" + response.status);
                assert(response.headers.has("Content-Type"), "Content-Type missing");
                const data = await response.json();
                assert(data.message === "busybar-js-fetch", "message=" + data.message);
                assert(data.value === 42, "value=" + data.value);
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript fetch reads the production update directory over HTTPS.")
    @pytest.mark.external_service
    def test_https_get_json(
        self,
        js_case_runner,
        device_wifi_ready,
    ):
        case_name = "https_get_json"
        url = json.dumps(UPDATE_DIRECTORY_URL)
        body = dedent(
            f"""
                async function fetchExternalJson(url) {{
                    const transientErrors = [];
                    for (let attempt = 1; attempt <= 4; attempt++) {{
                        try {{
                            const response = await fetch(url);
                            const directory = await response.json();
                            return {{response: response, directory: directory}};
                        }} catch (error) {{
                            const message = String(error);
                            transientErrors.push(message);
                            if (message !== "DNS error" &&
                                message !== "Inactivity timeout") {{
                                throw error;
                            }}
                            if (attempt < 4) {{
                                await new Promise(function(resolve) {{
                                    setTimeout(resolve, 2000);
                                }});
                            }}
                        }}
                    }}
                    throw new Error(
                        "external fetch retries exhausted: " +
                        transientErrors.join(", ")
                    );
                }}

                const result = await fetchExternalJson({url});
                const response = result.response;
                const directory = result.directory;
                assert(response.status === 200, "status=" + response.status);
                assert(response.ok === true, "ok=" + response.ok);
                assert(Array.isArray(directory.channels), "channels is not an array");
                assert(directory.channels.length > 0, "channels is empty");
                const channel = directory.channels[0];
                assert(typeof channel.id === "string", "channel.id is not a string");
                assert(Array.isArray(channel.versions), "versions is not an array");
            """
        ).strip()

        js_case_runner(case_name, body, timeout=60)

    @allure.title("JavaScript fetch rejects json() for malformed JSON.")
    def test_invalid_json_rejects(self, js_case_runner, http_server):
        case_name = "invalid_json"
        url = json.dumps(http_server.url("/invalid-json"))
        body = dedent(
            f"""
                const response = await fetch({url});
                let rejected = false;
                try {{
                    await response.json();
                }} catch (error) {{
                    rejected = true;
                }}
                assert(rejected, "json() accepted malformed JSON");
                assert(response.bodyUsed === true, "bodyUsed after json()");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript fetch rejects a call without a URL.")
    def test_missing_url_rejects(self, js_case_runner):
        case_name = "missing_url"
        body = dedent(
            """
                let rejected = false;
                try {
                    await fetch();
                } catch (error) {
                    rejected = true;
                }
                assert(rejected, "fetch() without URL was not rejected");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript fetch rejects an invalid init argument.")
    def test_invalid_init_rejects(self, js_case_runner, http_server):
        case_name = "invalid_init"
        url = json.dumps(http_server.url("/text"))
        body = dedent(
            f"""
                let rejected = false;
                try {{
                    await fetch({url}, 42);
                }} catch (error) {{
                    rejected = true;
                }}
                assert(rejected, "numeric init argument was not rejected");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript fetch rejects a disconnect before response headers.")
    def test_disconnect_before_headers_rejects(self, js_case_runner, http_server):
        case_name = "disconnect_before_headers"
        url = json.dumps(http_server.url("/disconnect"))
        body = dedent(
            f"""
                let rejected = false;
                try {{
                    await fetch({url});
                }} catch (error) {{
                    rejected = true;
                }}
                assert(rejected, "disconnect before headers was not rejected");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript fetch supports multiple concurrent requests.")
    @allure.issue("https://flipper.atlassian.net/browse/FW-1102")
    @pytest.mark.skip(reason="concurrent fetch returns socket error/inactivity timeout")
    def test_concurrent_requests(self, js_case_runner, http_server):
        case_name = "concurrent_requests"
        text_url = json.dumps(http_server.url("/text"))
        json_url = json.dumps(http_server.url("/json"))
        missing_url = json.dumps(http_server.url("/not-found"))
        expected_text = json.dumps(TEXT_PAYLOAD.decode("utf-8"))
        expected_missing = json.dumps(NOT_FOUND_PAYLOAD.decode("utf-8"))
        body = dedent(
            f"""
                const responses = await Promise.all([
                    fetch({text_url}),
                    fetch({json_url}),
                    fetch({missing_url})
                ]);
                assert(responses[0].status === 200,
                    "first status=" + responses[0].status);
                assert(responses[1].status === 200,
                    "second status=" + responses[1].status);
                assert(responses[2].status === 404,
                    "third status=" + responses[2].status);
                assert(await responses[0].text() === {expected_text}, "first body");
                const data = await responses[1].json();
                assert(data.value === 42, "JSON value=" + data.value);
                assert(await responses[2].text() === {expected_missing}, "third body");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript response headers support entries, keys, and values.")
    def test_headers_iterators(self, js_case_runner, http_server):
        case_name = "headers_iterators"
        url = json.dumps(http_server.url("/headers"))
        expected_text = json.dumps(HEADERS_PAYLOAD.decode("utf-8"))
        body = dedent(
            f"""
                const response = await fetch({url});
                let entryFound = false;
                for (const entry of response.headers.entries()) {{
                    if (entry[0] === "X-Fetch-Alpha" && entry[1] === "alpha") {{
                        entryFound = true;
                    }}
                }}
                let keyFound = false;
                for (const key of response.headers.keys()) {{
                    if (key === "X-Fetch-Number") keyFound = true;
                }}
                let valueFound = false;
                for (const value of response.headers.values()) {{
                    if (value === "42") valueFound = true;
                }}
                const text = await response.text();
                assert(text === {expected_text}, "response body");
                assert(entryFound, "X-Fetch-Alpha entry missing");
                assert(keyFound, "X-Fetch-Number key missing");
                assert(valueFound, "header value 42 missing");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript Headers.has is case-insensitive.")
    def test_headers_has_is_case_insensitive(self, js_case_runner, http_server):
        case_name = "headers_has_case_insensitive"
        url = json.dumps(http_server.url("/headers"))
        body = dedent(
            f"""
                const response = await fetch({url});
                const hasLowercaseName = response.headers.has("x-fetch-alpha");
                await response.text();
                assert(hasLowercaseName,
                    "lowercase header name was not found");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript Headers.forEach uses value-key-parent arguments.")
    def test_headers_for_each_argument_order(self, js_case_runner, http_server):
        case_name = "headers_foreach_order"
        url = json.dumps(http_server.url("/headers"))
        body = dedent(
            f"""
                const response = await fetch({url});
                let observedValue;
                let observedParent;
                response.headers.forEach(function(value, key, parent) {{
                    if (key === "X-Fetch-Alpha") {{
                        observedValue = value;
                        observedParent = parent;
                    }}
                }});
                await response.text();
                assert(observedValue !== undefined,
                    "X-Fetch-Alpha callback was not observed");
                assert(observedValue === "alpha", "value=" + observedValue);
                assert(observedParent === response.headers, "unexpected parent");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript Response.url contains the final request URL.")
    def test_response_url(self, js_case_runner, http_server):
        case_name = "response_url"
        raw_url = http_server.url("/text")
        url = json.dumps(raw_url)
        body = dedent(
            f"""
                const response = await fetch({url});
                await response.text();
                assert(response.url === {url}, "url=" + response.url);
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript fetch exposes a 404 response and its body.")
    def test_http_404_resolves(self, js_case_runner, http_server):
        case_name = "http_404"
        url = json.dumps(http_server.url("/not-found"))
        expected_text = json.dumps(NOT_FOUND_PAYLOAD.decode("utf-8"))
        body = dedent(
            f"""
                const response = await fetch({url});
                assert(response.status === 404, "status=" + response.status);
                assert(response.ok === false, "ok=" + response.ok);
                assert(await response.text() === {expected_text}, "response body");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript fetch exposes a successful HTTP 201 response.")
    def test_http_201_created(self, js_case_runner, http_server):
        case_name = "http_201_created"
        url = json.dumps(http_server.url("/created"))
        body = dedent(
            f"""
                const response = await fetch({url});
                assert(response.status === 201, "status=" + response.status);
                assert(response.statusText === "Created",
                    "statusText=" + response.statusText);
                assert(response.ok === true, "ok=" + response.ok);
                const data = await response.json();
                assert(data.created === true, "created=" + data.created);
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript fetch resolves an HTTP 500 response.")
    def test_http_500_resolves(self, js_case_runner, http_server):
        case_name = "http_500"
        url = json.dumps(http_server.url("/server-error"))
        body = dedent(
            f"""
                const response = await fetch({url});
                assert(response.status === 500, "status=" + response.status);
                assert(response.statusText === "Internal Server Error",
                    "statusText=" + response.statusText);
                assert(response.ok === false, "ok=" + response.ok);
                const data = await response.json();
                assert(data.error === "server-error", "error=" + data.error);
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript fetch exposes an HTTP 204 response with an empty body.")
    def test_empty_body(self, js_case_runner, http_server):
        case_name = "empty_body"
        url = json.dumps(http_server.url("/empty"))
        body = dedent(
            f"""
                const response = await fetch({url});
                assert(response.status === 204, "status=" + response.status);
                assert(response.statusText === "No Content",
                    "statusText=" + response.statusText);
                assert(response.ok === true, "ok=" + response.ok);
                const text = await response.text();
                assert(text === "", "text length=" + text.length);
                assert(response.bodyUsed === true, "bodyUsed after read");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript response body can only be consumed once.")
    def test_body_is_single_use(self, js_case_runner, http_server):
        case_name = "body_single_use"
        url = json.dumps(http_server.url("/text"))
        body = dedent(
            f"""
                const response = await fetch({url});
                await response.text();
                let rejected = false;
                try {{
                    await response.text();
                }} catch (error) {{
                    rejected = true;
                }}
                assert(rejected, "second body read was not rejected");
                assert(response.bodyUsed === true, "bodyUsed after read");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript fetch rejects a truncated text response.")
    def test_truncated_body_rejects(self, js_case_runner, http_server):
        case_name = "truncated_body"
        url = json.dumps(http_server.url("/truncated"))
        body = dedent(
            f"""
                const response = await fetch({url});
                let rejected = false;
                try {{
                    await response.text();
                }} catch (error) {{
                    rejected = true;
                }}
                assert(rejected, "truncated body was not rejected");
            """
        ).strip()

        js_case_runner(case_name, body)

    @allure.title("JavaScript fetch rejects inactivity after response headers.")
    def test_inactivity_after_headers_rejects(self, js_case_runner, http_server):
        case_name = "inactivity_after_headers"
        url = json.dumps(http_server.url("/stall"))
        body = dedent(
            f"""
                const response = await fetch({url});
                let rejected = false;
                try {{
                    await response.text();
                }} catch (error) {{
                    rejected = true;
                }}
                assert(rejected, "inactivity timeout did not reject body read");
            """
        ).strip()

        js_case_runner(case_name, body, timeout=12)

    @allure.title("JavaScript fetch accepts the documented ten request headers.")
    def test_ten_request_headers(self, js_case_runner, http_server):
        case_name = "ten_request_headers"
        url = json.dumps(http_server.url("/request"))
        headers = {f"X-JS-Fetch-{index}": str(index) for index in range(10)}
        body = dedent(
            f"""
                const response = await fetch({url}, {{headers: {json.dumps(headers)}}});
                assert(response.status === 200, "status=" + response.status);
                await response.text();
            """
        ).strip()

        output = js_case_runner(case_name, body)

        with allure.step("Verify all ten headers reached the host server"):
            captured = captured_request(http_server, output, "ten-header request")
            for index in range(10):
                name = f"x-js-fetch-{index}"
                actual = captured["headers"].get(name)
                assert actual == str(index), (
                    f"expected {name}={index}, got {actual!r}; "
                    f"captured={captured!r}, output={output!r}"
                )

    @allure.title("JavaScript fetch releases resources across repeated requests.")
    @pytest.mark.long_running
    def test_repeated_requests(self, js_case_runner, http_server):
        case_name = "repeated_requests"
        url = json.dumps(http_server.url("/text"))
        body = dedent(
            f"""
                for (let index = 0; index < 25; index++) {{
                    const response = await fetch({url});
                    assert(response.status === 200,
                        "iteration=" + index + ", status=" + response.status);
                    const text = await response.text();
                    assert(text.length > 0, "empty body at iteration=" + index);
                }}
            """
        ).strip()

        js_case_runner(case_name, body, timeout=60)
