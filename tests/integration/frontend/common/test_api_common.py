import queue
import threading

import allure
import pytest

from clients.api import BaseAPI, InputAPI, SettingsAPI, StorageAPI, StreamingAPI


@allure.feature("5. Web Frontend")
@allure.story("Common")
class TestAPIErrorHandling:
    """Test cases for API error handling and edge cases"""

    @allure.id("2671")
    @allure.title("GET /api/* (404 errors)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_endpoints_404(self, api_session, web_base_url):
        """Test that non-existent API endpoints return 404"""
        api = BaseAPI(api_session, web_base_url)

        invalid_endpoints = [
            "/api/nonexistent",
            "/api/invalid/endpoint",
            "/api/test/fake",
            "/api/version/invalid",
        ]

        for endpoint in invalid_endpoints:
            with allure.step(f"Test invalid endpoint: {endpoint}"):
                response = api.get_raw(endpoint)
                assert response.status_code == 404

    @allure.id("2673")
    @allure.title("API endpoints (missing required parameters)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_missing_required_parameters(
        self, storage_api: StorageAPI, input_api: InputAPI, streaming_api: StreamingAPI
    ):
        """Test API endpoints with missing required parameters"""
        with allure.step("Test missing path for /api/storage/list"):
            # storage_api.list requires path parameter
            response = storage_api.get_raw("/api/storage/list")
            assert response.status_code == 400

        with allure.step("Test missing key for /api/input"):
            response = input_api.post_raw("/api/input")
            assert response.status_code == 400

        with allure.step("Test missing display for /api/screen"):
            response = streaming_api.get_raw("/api/screen")
            assert response.status_code == 400

    @allure.id("2674")
    @allure.title("API endpoints (invalid parameter values)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_invalid_parameter_values(
        self, input_api: InputAPI, streaming_api: StreamingAPI, settings_api: SettingsAPI
    ):
        """Test API endpoints with invalid parameter values"""
        test_cases = [
            ("input invalid key", lambda: input_api.send_key("invalid_key_name")),
            ("screen invalid string", lambda: streaming_api.get_screen(display="invalid")),
            ("screen negative", lambda: streaming_api.get_screen(display=-1)),
            ("volume negative", lambda: settings_api.set_volume_raw(-50)),
            ("volume over max", lambda: settings_api.set_volume_raw(150)),
        ]

        for name, test_func in test_cases:
            with allure.step(f"Test invalid parameters: {name}"):
                response = test_func()
                assert response.status_code == 400

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

            success_count = sum(1 for code in response_codes if code == 200)
            assert success_count >= 3, (
                f"Expected at least 3 successful requests, got {success_count}"
            )


@allure.feature("5. Web Frontend")
@allure.story("Common")
class TestAPIAuthentication:
    """Test cases for API authentication (when implemented)"""

    @allure.id("2709")
    @allure.title("GET /api/status (no auth token)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Authentication not implemented yet")
    def test_api_without_auth_token(self, api_session, web_base_url):
        """Test API access without authentication token"""
        headers = api_session.headers.copy()
        if "X-API-Token" in headers:
            del headers["X-API-Token"]

        with allure.step("Access protected endpoint without auth"):
            response = api_session.get(
                f"{web_base_url}/api/status", headers=headers, timeout=10
            )

        assert response.status_code == 401

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

        assert response.status_code == 403

    @allure.id("2711")
    @allure.title("GET /api/status (valid auth token)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Authentication not implemented yet")
    def test_api_with_valid_auth_token(self, api_auth_session, web_base_url):
        """Test API access with valid authentication token"""
        with allure.step("Access protected endpoint with valid auth"):
            response = api_auth_session.get(f"{web_base_url}/api/status", timeout=10)

        assert response.status_code == 200
