import time

import allure
import pytest
import requests
from bs4 import BeautifulSoup


@allure.epic("BSB Web Frontend Testing")
@allure.feature("5. Web Frontend")
@allure.story("Basic connectivity")
class TestWebFrontendBasic:
    """Basic smoke tests - HTML page and API documentation"""

    @allure.testcase("2001", "BSB Front. HTTP Server Response")
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

            # Check for basic HTML structure
            assert soup.find("html") is not None, "Missing <html> tag"
            assert soup.find("head") is not None, "Missing <head> tag"
            assert soup.find("body") is not None, "Missing <body> tag"

            # Check for Nuxt app container (this is what actually matters)
            app_container = soup.find("div", {"id": "__nuxt"})
            assert app_container is not None, "Missing Nuxt app container"

    @allure.testcase("2002", "BSB Front. API Documentation Available")
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

            # Check for Swagger UI
            swagger_container = soup.find("div", {"id": "swagger-ui"})
            assert swagger_container is not None, "Missing Swagger UI container"

    @allure.testcase("2003", "BSB Front. OpenAPI Spec Available")
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
            assert "Busy Bar HTTP API" in content, "Should contain API title"
            assert "paths:" in content, "Should contain API paths"


@allure.epic("BSB Web Frontend Testing")
@allure.feature("5. Web Frontend")
@allure.story("API endpoints")
class TestWebFrontendAPI:
    """Test actual API endpoints from the documentation"""

    @allure.testcase("2010", "BSB API. Version Endpoint")
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

            # Try to parse JSON
            try:
                version_data = response.json()
                allure.attach(
                    str(version_data), "Version Data", allure.attachment_type.JSON
                )
            except ValueError:
                pytest.fail("Response is not valid JSON")

    @allure.testcase("2011", "BSB API. Status Endpoint")
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

    @allure.testcase("2012", "BSB API. WiFi Status Endpoint")
    @pytest.mark.frontend
    def test_api_wifi_status_endpoint(self, web_session, web_base_url):
        """Test /api/wifi/status endpoint responds"""
        response = web_session.get(f"{web_base_url}/api/wifi/status", timeout=10)

        with allure.step("Verify wifi status endpoint"):
            # Should return 200 or possibly 404/405 if not implemented
            assert response.status_code in [
                200,
                404,
                405,
            ], f"Got unexpected status {response.status_code}"

            if response.status_code == 200:
                assert (
                    "application/json"
                    in response.headers.get("content-type", "").lower()
                )
                try:
                    wifi_data = response.json()
                    allure.attach(
                        str(wifi_data), "WiFi Status Data", allure.attachment_type.JSON
                    )
                except ValueError:
                    pytest.fail("Response is not valid JSON")

    @allure.testcase("2013", "BSB API. System Status Endpoint")
    @pytest.mark.frontend
    def test_api_system_status_endpoint(self, web_session, web_base_url):
        """Test /api/status/system endpoint responds"""
        response = web_session.get(f"{web_base_url}/api/status/system", timeout=10)

        with allure.step("Verify system status endpoint"):
            assert response.status_code in [
                200,
                404,
                405,
            ], f"Got unexpected status {response.status_code}"

            if response.status_code == 200:
                assert (
                    "application/json"
                    in response.headers.get("content-type", "").lower()
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


@allure.epic("BSB Web Frontend Testing")
@allure.feature("5. Web Frontend")
@allure.story("Error handling")
class TestWebFrontendErrorHandling:
    """Test error handling and edge cases"""

    @allure.testcase("2020", "BSB Front. 404 Handling")
    @pytest.mark.frontend
    def test_404_handling(self, web_session, web_base_url):
        """Test that non-existent paths return 404"""
        response = web_session.get(f"{web_base_url}/nonexistent-page", timeout=10)

        with allure.step("Verify 404 response"):
            assert (
                response.status_code == 404
            ), f"Expected 404, got {response.status_code}"

    @allure.testcase("2021", "BSB Front. Invalid API Path")
    @pytest.mark.frontend
    def test_invalid_api_path(self, web_session, web_base_url):
        """Test invalid API endpoint returns appropriate error"""
        response = web_session.get(f"{web_base_url}/api/invalid-endpoint", timeout=10)

        with allure.step("Verify error response"):
            assert response.status_code in [
                404,
                405,
            ], f"Expected 404/405, got {response.status_code}"

    @allure.testcase("2022", "BSB Front. Malicious Query Parameters")
    @pytest.mark.frontend
    def test_malicious_query_parameters(self, web_session, web_base_url):
        """Test that server handles malicious query parameters gracefully"""
        malicious_params = [
            "?param=<script>alert('xss')</script>",
            "?param=" + "A" * 10000,  # Very long parameter
            "?param=../../../etc/passwd",
            "?param='; DROP TABLE users; --",
        ]

        for malicious_param in malicious_params:
            with allure.step(f"Test parameter: {malicious_param[:50]}..."):
                response = web_session.get(
                    f"{web_base_url}/{malicious_param}", timeout=10
                )
                # Should not crash the server - any response code is fine as long as we get one
                assert (
                    response.status_code is not None
                ), "Server should respond to malicious requests"
                assert (
                    response.status_code < 500
                ), f"Server error with param {malicious_param[:20]}: {response.status_code}"

    @pytest.mark.parametrize(
        "endpoint",
        ["/api/version", "/api/status", "/api/wifi/status", "/docs/", "/openapi.yaml"],
    )
    @allure.testcase("2023", "BSB Front. Response Time Test")
    @pytest.mark.frontend
    def test_response_times(self, web_session, web_base_url, endpoint):
        """Test that API endpoints respond within reasonable time"""
        start_time = time.time()
        response = web_session.get(f"{web_base_url}{endpoint}", timeout=10)
        response_time = time.time() - start_time

        with allure.step(f"Check response time for {endpoint}"):
            # Allow some endpoints to be not implemented
            assert response.status_code in [
                200,
                404,
                405,
            ], f"Unexpected status for {endpoint}: {response.status_code}"
            assert (
                response_time < 5.0
            ), f"Response too slow for {endpoint}: {response_time:.2f}s"

            allure.attach(
                f"Endpoint: {endpoint}\nStatus: {response.status_code}\nTime: {response_time:.3f}s",
                "Response Info",
                allure.attachment_type.TEXT,
            )


@allure.epic("BSB Web Frontend Testing")
@allure.feature("5. Web Frontend")
@allure.story("Integration")
class TestWebFrontendIntegration:
    """Integration tests - multiple components working together"""

    @allure.testcase("2030", "BSB Front. Complete Stack Test")
    @pytest.mark.frontend
    def test_complete_stack_functional(self, web_session, web_base_url):
        """Test that web interface, API docs, and core API endpoints all work together"""

        # Test main page
        main_response = web_session.get(web_base_url, timeout=10)
        assert main_response.status_code == 200, "Main page should load"

        # Test API docs
        docs_response = web_session.get(f"{web_base_url}/docs/", timeout=10)
        assert docs_response.status_code == 200, "API docs should load"

        # Test at least one API endpoint works
        api_response = web_session.get(f"{web_base_url}/api/version", timeout=10)
        assert api_response.status_code == 200, "At least one API endpoint should work"

        # Test OpenAPI spec
        spec_response = web_session.get(f"{web_base_url}/openapi.yaml", timeout=10)
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
            allure.attach(summary, "Stack Status Summary", allure.attachment_type.TEXT)
