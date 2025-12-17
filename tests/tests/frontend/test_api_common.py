import json
import queue
import threading

import allure
import pytest


@allure.feature("5. Web Frontend")
@allure.story("Local API - Common")
class TestAPIErrorHandling:
    """Test cases for API error handling and edge cases"""

    @allure.id("2671")
    @allure.title("GET /api/* (404 errors)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_endpoints_404(self, api_session, web_base_url):
        """Test that non-existent API endpoints return 404"""

        invalid_endpoints = [
            "/api/nonexistent",
            "/api/invalid/endpoint",
            "/api/test/fake",
            "/api/version/invalid",
        ]

        for endpoint in invalid_endpoints:
            with allure.step(f"Test invalid endpoint: {endpoint}"):
                response = api_session.get(f"{web_base_url}{endpoint}", timeout=10)
                assert (
                    response.status_code == 404
                ), f"Expected 404 for {endpoint}, got {response.status_code}"

    @allure.id("2673")
    @allure.title("API endpoints (missing required parameters)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_missing_required_parameters(self, api_session, web_base_url):
        """Test API endpoints with missing required parameters"""

        test_cases = [
            ("/api/storage/list", {}),  # Missing path parameter
            ("/api/input", {}),  # Missing key parameter
            ("/api/screen", {}),  # Missing display parameter
        ]

        for endpoint, params in test_cases:
            with allure.step(f"Test missing parameters for {endpoint}"):
                if endpoint == "/api/input":
                    response = api_session.post(
                        f"{web_base_url}{endpoint}", params=params, timeout=10
                    )
                else:
                    response = api_session.get(
                        f"{web_base_url}{endpoint}", params=params, timeout=10
                    )

                assert (
                    response.status_code == 400
                ), f"Expected 400 for missing params on {endpoint}, got {response.status_code}"

    @allure.id("2674")
    @allure.title("API endpoints (invalid parameter values)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_invalid_parameter_values(self, api_session, web_base_url):
        """Test API endpoints with invalid parameter values"""

        test_cases = [
            ("/api/input", {"key": "invalid_key_name"}),
            ("/api/screen", {"display": "invalid"}),
            ("/api/screen", {"display": -1}),
            ("/api/audio/volume", {"volume": -50}),
            ("/api/audio/volume", {"volume": 150}),
        ]

        for endpoint, params in test_cases:
            with allure.step(f"Test invalid parameters for {endpoint}"):
                if endpoint == "/api/input":
                    response = api_session.post(
                        f"{web_base_url}{endpoint}", params=params, timeout=10
                    )
                elif endpoint == "/api/audio/volume":
                    response = api_session.post(
                        f"{web_base_url}{endpoint}", params=params, timeout=10
                    )
                else:
                    response = api_session.get(
                        f"{web_base_url}{endpoint}", params=params, timeout=10
                    )

                assert (
                    response.status_code == 400
                ), f"Expected 400 for invalid params on {endpoint}, got {response.status_code}"

    @allure.id("2708")
    @allure.title("GET /api/version (concurrent requests)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_concurrent_requests(self, api_session, web_base_url):
        """Test API behavior under concurrent requests"""

        results = queue.Queue()

        def make_request():
            try:
                response = api_session.get(f"{web_base_url}/api/version", timeout=10)
                results.put(response.status_code)
            except Exception as e:
                results.put(f"Error: {e}")

        with allure.step("Send 5 concurrent requests"):
            threads = []
            for i in range(5):
                thread = threading.Thread(target=make_request)
                threads.append(thread)
                thread.start()

            for thread in threads:
                thread.join()

        with allure.step("Verify all requests succeeded"):
            response_codes = []
            while not results.empty():
                result = results.get()
                response_codes.append(result)

            # All requests should succeed (status 200)
            success_count = sum(1 for code in response_codes if code == 200)
            assert (
                success_count >= 3
            ), f"Expected at least 3 successful requests, got {success_count} out of {len(response_codes)}"


@allure.feature("5. Web Frontend")
@allure.story("Local API - Common")
class TestAPIAuthentication:
    """Test cases for API authentication (when implemented)"""

    @allure.id("2709")
    @allure.title("GET /api/status (no auth token)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Authentication not implemented yet")
    def test_api_without_auth_token(self, api_session, web_base_url):
        """Test API access without authentication token"""

        # Remove any auth headers
        headers = api_session.headers.copy()
        if "X-API-Token" in headers:
            del headers["X-API-Token"]

        with allure.step("Access protected endpoint without auth"):
            response = api_session.get(
                f"{web_base_url}/api/status", headers=headers, timeout=10
            )

        with allure.step("Verify authentication required"):
            assert (
                response.status_code == 401
            ), f"Expected 401 Unauthorized, got {response.status_code}"

    @allure.id("2710")
    @allure.title("GET /api/status (invalid auth token)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Authentication not implemented yet")
    def test_api_with_invalid_auth_token(self, api_session, web_base_url):
        """Test API access with invalid authentication token"""

        headers = api_session.headers.copy()
        headers["X-API-Token"] = "invalid-token"

        with allure.step("Access protected endpoint with invalid auth"):
            response = api_session.get(
                f"{web_base_url}/api/status", headers=headers, timeout=10
            )

        with allure.step("Verify invalid token rejection"):
            assert (
                response.status_code == 403
            ), f"Expected 403 Forbidden, got {response.status_code}"

    @allure.id("2711")
    @allure.title("GET /api/status (valid auth token)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Authentication not implemented yet")
    def test_api_with_valid_auth_token(self, api_auth_session, web_base_url):
        """Test API access with valid authentication token"""

        with allure.step("Access protected endpoint with valid auth"):
            response = api_auth_session.get(f"{web_base_url}/api/status", timeout=10)

        with allure.step("Verify successful access"):
            assert (
                response.status_code == 200
            ), f"Expected 200 with valid auth, got {response.status_code}"
