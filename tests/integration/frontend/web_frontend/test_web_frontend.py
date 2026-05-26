import gzip as gzip_module
import http.client
import time
from urllib.parse import urlparse

import allure
import pytest
from bs4 import BeautifulSoup


@allure.feature("5. Web Frontend")
@allure.story("Basic connectivity")
class TestWebFrontendBasic:
    """Basic smoke tests - HTML page and API documentation"""

    @allure.id("2728")
    @allure.title("BSB Front. HTTP Server Response")
    @pytest.mark.frontend
    def test_http_server_responds(self, web_session, web_base_url):
        """Test that the HTTP server responds with valid HTML"""
        response = web_session.get(web_base_url, timeout=10)

        with allure.step("Verify HTTP response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

        with allure.step("Verify content type"):
            content_type = response.headers.get("content-type", "").lower()
            assert "text/html" in content_type, f"Expected HTML, got {content_type}"

        with allure.step("Verify HTML structure"):
            soup = BeautifulSoup(response.text, "html.parser")

            assert soup.find("html") is not None, "Missing <html> tag"
            assert soup.find("head") is not None, "Missing <head> tag"
            assert soup.find("body") is not None, "Missing <body> tag"

            app_container = soup.find("div", {"id": "__nuxt"})
            assert app_container is not None, "Missing Nuxt app container"

    @allure.id("2729")
    @allure.title("BSB Front. API Documentation Available")
    @pytest.mark.frontend
    def test_api_docs_available(self, web_session, web_base_url):
        """Test that Swagger API documentation is accessible"""
        docs_url = f"{web_base_url}/docs/"
        response = web_session.get(docs_url, timeout=10)

        with allure.step("Verify docs page loads"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

        with allure.step("Verify API documentation content"):
            soup = BeautifulSoup(response.text, "html.parser")
            title = soup.find("title")
            assert title is not None, "Missing docs title"
            assert (
                "BSB Firmware API Documentation" in title.text
            ), f"Expected API docs title, got: {title.text}"

            swagger_container = soup.find("div", {"id": "swagger-ui"})
            assert swagger_container is not None, "Missing Swagger UI container"

    @allure.id("2730")
    @allure.title("BSB Front. OpenAPI Spec Available")
    @pytest.mark.frontend
    def test_openapi_spec_available(self, web_session, web_base_url):
        """Test that OpenAPI specification is accessible"""
        openapi_url = f"{web_base_url}/openapi.yaml"
        response = web_session.get(openapi_url, timeout=10)

        with allure.step("Verify OpenAPI spec loads"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

        with allure.step("Verify YAML content"):
            content = response.text
            assert "openapi:" in content, "Should contain OpenAPI version"
            assert "title: BUSY Bar HTTP API" in content, "Should contain API title"
            assert "paths:" in content, "Should contain API paths"


@allure.feature("5. Web Frontend")
@allure.story("API endpoints")
class TestWebFrontendAPI:
    """Test actual API endpoints from the documentation"""

    @allure.id("2731")
    @allure.title("BSB API. Version Endpoint")
    @pytest.mark.frontend
    def test_api_version_endpoint(self, web_session, web_base_url):
        """Test /api/version endpoint responds"""
        response = web_session.get(f"{web_base_url}/api/version", timeout=10)

        with allure.step("Verify version endpoint responds"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

        with allure.step("Verify JSON response"):
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            try:
                version_data = response.json()
                allure.attach(
                    str(version_data), "Version Data", allure.attachment_type.JSON
                )
            except ValueError:
                pytest.fail("Response is not valid JSON")

    @allure.id("2732")
    @allure.title("BSB API. Status Endpoint")
    @pytest.mark.frontend
    def test_api_status_endpoint(self, web_session, web_base_url):
        """Test /api/status endpoint responds"""
        response = web_session.get(f"{web_base_url}/api/status", timeout=10)

        with allure.step("Verify status endpoint responds"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

        with allure.step("Verify JSON response"):
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            try:
                status_data = response.json()
                allure.attach(
                    str(status_data), "Status Data", allure.attachment_type.JSON
                )
            except ValueError:
                pytest.fail("Response is not valid JSON")

    @allure.id("2733")
    @allure.title("BSB API. WiFi Status Endpoint")
    @pytest.mark.frontend
    def test_api_wifi_status_endpoint(self, web_session, web_base_url):
        """Test /api/wifi/status endpoint responds"""
        response = web_session.get(f"{web_base_url}/api/wifi/status", timeout=10)

        with allure.step("Verify wifi status endpoint"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )
            try:
                wifi_data = response.json()
                allure.attach(
                    str(wifi_data), "WiFi Status Data", allure.attachment_type.JSON
                )
            except ValueError:
                pytest.fail("Response is not valid JSON")

    @allure.id("2734")
    @allure.title("BSB API. System Status Endpoint")
    @pytest.mark.frontend
    def test_api_system_status_endpoint(self, web_session, web_base_url):
        """Test /api/status/system endpoint responds"""
        response = web_session.get(f"{web_base_url}/api/status/system", timeout=10)

        with allure.step("Verify system status endpoint"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )
            try:
                system_data = response.json()
                allure.attach(
                    str(system_data),
                    "System Status Data",
                    allure.attachment_type.JSON,
                )
            except ValueError:
                pytest.fail("Response is not valid JSON")


@allure.feature("5. Web Frontend")
@allure.story("Error handling")
class TestWebFrontendErrorHandling:
    """Test error handling and edge cases"""

    @allure.id("2735")
    @allure.title("BSB Front. 404 Handling")
    @pytest.mark.frontend
    def test_404_handling(self, web_session, web_base_url):
        """Test that non-existent paths return 404 with the custom HTML page"""

        response = web_session.get(f"{web_base_url}/nonexistent-page", timeout=10)

        with allure.step("Verify 404 status code"):
            assert (
                response.status_code == 404
            ), f"Expected 404, got {response.status_code}"

        with allure.step("Verify 404 page is HTML"):
            content_type = response.headers.get("content-type", "").lower()
            assert (
                "text/html" in content_type
            ), f"Expected HTML 404 page, got {content_type}"

        with allure.step("Verify 404 page body"):
            assert "404" in response.text, "404 page body should contain '404'"

    @allure.id("2736")
    @allure.title("BSB Front. Invalid API Path")
    @pytest.mark.frontend
    def test_invalid_api_path(self, web_session, web_base_url):
        """Test invalid API endpoint returns appropriate error"""
        response = web_session.get(f"{web_base_url}/api/invalid-endpoint", timeout=10)

        with allure.step("Verify error response"):
            assert response.status_code in [
                404,
                405,
            ], f"Expected 404/405, got {response.status_code}"

    @allure.id("2738")
    @pytest.mark.parametrize(
        "endpoint",
        ["/api/version", "/api/status", "/api/wifi/status", "/docs/", "/openapi.yaml"],
    )
    @allure.title("BSB Front. Response Time Test")
    @pytest.mark.frontend
    def test_response_times(self, web_session, web_base_url, endpoint):
        """Test that API endpoints respond within reasonable time"""
        start_time = time.time()
        response = web_session.get(f"{web_base_url}{endpoint}", timeout=10)
        response_time = time.time() - start_time

        with allure.step(f"Check response time for {endpoint}"):
            assert (
                response.status_code == 200
            ), f"Expected 200 for {endpoint}, got {response.status_code}"
            assert (
                response_time < 5.0
            ), f"Response too slow for {endpoint}: {response_time:.2f}s"

            allure.attach(
                f"Endpoint: {endpoint}\nStatus: {response.status_code}\nTime: {response_time:.3f}s",
                name="Response Info",
                attachment_type=allure.attachment_type.TEXT
            )


@allure.feature("5. Web Frontend")
@allure.story("Gzip content negotiation")
class TestWebFrontendGzip:
    """Tests for gzip content negotiation on pre-compressed static assets.

    The web assets are stored exclusively as .gz files. The server must:
    - Case A: serve gzip-encoded content when client omits Accept-Encoding (RFC 7231 §5.3.4)
    - Case B: serve gzip-encoded content when client advertises gzip support (baseline)
    - Case C: return 406 for compressed-only resources when client opts out of gzip
    - Case C (uncompressed): return 200 for resources that are not compressed
    """

    @allure.title("BSB Front. No Accept-Encoding serves gzip (Case A)")
    @pytest.mark.frontend
    def test_no_accept_encoding_serves_gzip(self, web_base_url):
        """Without Accept-Encoding the server injects gzip and serves the asset.

        Uses http.client with skip_accept_encoding=True to truly omit the header;
        requests/urllib3 v2 and stdlib http.client.request() both inject
        'Accept-Encoding: identity' when no encoding is specified by the caller.
        """
        parsed = urlparse(web_base_url)
        host = parsed.hostname
        port = parsed.port or 80
        path = parsed.path or "/"

        conn = http.client.HTTPConnection(host, port, timeout=10)
        conn.connect()
        # skip_accept_encoding=True prevents http.client from adding the default
        # 'Accept-Encoding: identity', giving us a request with no AE header at all
        conn.putrequest("GET", path, skip_host=True, skip_accept_encoding=True)
        conn.putheader("Host", parsed.netloc)
        conn.putheader("User-Agent", "test-client")
        conn.endheaders()
        resp = conn.getresponse()
        body = resp.read()
        conn.close()

        with allure.step("Verify 200 response"):
            assert resp.status == 200, f"Expected 200, got {resp.status}"

        with allure.step("Verify Content-Encoding: gzip header is present"):
            content_encoding = resp.getheader("Content-Encoding", "")
            assert (
                content_encoding.lower() == "gzip"
            ), f"Expected Content-Encoding: gzip, got: {content_encoding}"

        with allure.step("Verify HTML content is decompressed correctly"):
            decompressed = gzip_module.decompress(body).decode("utf-8", errors="replace")
            assert "html" in decompressed.lower(), "Decompressed response should contain HTML"

    @allure.title("BSB Front. Accept-Encoding: gzip serves gzip (Case B)")
    @pytest.mark.frontend
    def test_explicit_gzip_serves_gzip(self, web_session, web_base_url):
        """Explicit Accept-Encoding: gzip results in 200 with Content-Encoding: gzip."""
        response = web_session.get(
            web_base_url,
            headers={"Accept-Encoding": "gzip"},
        )

        with allure.step("Verify 200 response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

        with allure.step("Verify Content-Encoding: gzip header is present"):
            assert (
                response.headers.get("Content-Encoding", "").lower() == "gzip"
            ), f"Expected Content-Encoding: gzip, got: {response.headers.get('Content-Encoding')}"

    @allure.title(
        "BSB Front. Accept-Encoding: identity returns 406 for compressed asset (Case C)"
    )
    @pytest.mark.frontend
    def test_identity_only_returns_406_for_compressed_asset(self, web_session, web_base_url):
        """When gzip is excluded, a compressed-only asset returns 406 Not Acceptable."""
        response = web_session.get(
            web_base_url,
            headers={"Accept-Encoding": "identity"},
        )

        with allure.step("Verify 406 response"):
            assert (
                response.status_code == 406
            ), f"Expected 406 Not Acceptable, got {response.status_code}"

    @allure.id("2743")
    @allure.title(
        "BSB Front. Accept-Encoding: identity serves uncompressed asset (Case C, uncompressed)"
    )
    @pytest.mark.frontend
    def test_identity_only_serves_uncompressed_asset(self, web_session, web_base_url):
        """When gzip is excluded, an uncompressed asset (e.g. .yaml) is served normally."""
        response = web_session.get(
            f"{web_base_url}/openapi.yaml",
            headers={"Accept-Encoding": "identity"},
        )

        with allure.step("Verify 200 response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

        with allure.step("Verify no Content-Encoding header"):
            assert (
                "Content-Encoding" not in response.headers
            ), f"Unexpected Content-Encoding: {response.headers.get('Content-Encoding')}"


@allure.feature("5. Web Frontend")
@allure.story("Integration")
class TestWebFrontendIntegration:
    """Integration tests - multiple components working together"""

    @allure.id("2739")
    @allure.title("BSB Front. Complete Stack Test")
    @pytest.mark.frontend
    def test_complete_stack_functional(self, web_session, web_base_url):
        """Test that web interface, API docs, and core API endpoints all work together"""

        main_response = web_session.get(web_base_url, timeout=5)
        assert main_response.status_code == 200, "Main page should load"

        docs_response = web_session.get(f"{web_base_url}/docs/", timeout=5)
        assert docs_response.status_code == 200, "API docs should load"

        api_response = web_session.get(f"{web_base_url}/api/version", timeout=5)
        assert api_response.status_code == 200, "At least one API endpoint should work"

        spec_response = web_session.get(f"{web_base_url}/openapi.yaml", timeout=5)
        assert spec_response.status_code == 200, "OpenAPI spec should be available"

        with allure.step("Verify complete stack is functional"):
            all_working = [
                main_response.status_code == 200,
                docs_response.status_code == 200,
                api_response.status_code == 200,
                spec_response.status_code == 200,
            ]
            assert all(all_working), "All major components should be functional"

            summary = f"""
            Main page: {main_response.status_code}
            API docs: {docs_response.status_code}
            Version API: {api_response.status_code}
            OpenAPI spec: {spec_response.status_code}
            """
            allure.attach(summary, name="Stack Status Summary", attachment_type=allure.attachment_type.TEXT)
