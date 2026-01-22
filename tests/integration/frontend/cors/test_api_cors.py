import allure
import pytest

from clients.api import BaseAPI


# All API endpoints from OpenAPI spec
API_ENDPOINTS = [
    "/api/version",
    "/api/access",
    "/api/name",
    "/api/update",
    "/api/update/check",
    "/api/update/status",
    "/api/update/changelog",
    "/api/update/install",
    "/api/update/abort_download",
    "/api/assets/upload",
    "/api/storage/write",
    "/api/storage/read",
    "/api/storage/list",
    "/api/storage/remove",
    "/api/storage/mkdir",
    "/api/storage/status",
    "/api/display/draw",
    "/api/display/brightness",
    "/api/audio/play",
    "/api/audio/volume",
    pytest.param("/api/input", marks=pytest.mark.xfail(reason="CORS preflight not supported")),
    "/api/status",
    "/api/status/system",
    "/api/status/power",
    "/api/wifi/status",
    "/api/wifi/connect",
    "/api/wifi/disconnect",
    "/api/wifi/networks",
    pytest.param("/api/screen", marks=pytest.mark.xfail(reason="CORS preflight not supported")),
    "/api/screen/ws",
    "/api/ble/enable",
    "/api/ble/disable",
    "/api/ble/pairing",
    "/api/ble/status",
    "/api/time",
    "/api/account",
    "/api/account/link",
    "/api/account/info",
    "/api/account/status",
    pytest.param("/api/account/profile", marks=pytest.mark.xfail(reason="CORS preflight not supported")),
    "/api/time/timestamp",
    "/api/time/timezone",
    "/api/busy/snapshot",
]


@allure.feature("5. Web Frontend")
@allure.story("CORS")
class TestAPICors:
    """Test cases for CORS (Cross-Origin Resource Sharing) support"""

    @allure.id("2750")
    @allure.title("OPTIONS preflight requests")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("endpoint", API_ENDPOINTS)
    def test_cors_preflight_options(self, api_session, web_base_url, endpoint):
        """
        Test that OPTIONS preflight requests work for all API endpoints.

        CORS preflight requests are sent by browsers before making cross-origin
        requests. The server should respond with appropriate CORS headers.
        """
        api = BaseAPI(api_session, web_base_url)

        with allure.step(f"Send OPTIONS request to {endpoint}"):
            headers = {
                "Origin": "http://busybar.local",
                "Access-Control-Request-Method": "GET",
            }
            response = api.options(endpoint, headers=headers)

        with allure.step("Verify OPTIONS response is successful"):
            assert response.status_code in [200, 204], (
                f"OPTIONS {endpoint} failed with status {response.status_code}. "
                f"Expected 200 or 204 for CORS preflight."
            )

    @allure.id("2751")
    @allure.title("OPTIONS /api/name (CORS preflight)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_cors_preflight_api_name(self, api_session, web_base_url):
        """
        Test OPTIONS preflight request specifically for /api/name endpoint.

        This is a regression test for the bug where OPTIONS /api/name fails.
        """
        api = BaseAPI(api_session, web_base_url)
        endpoint = "/api/name"

        with allure.step("Send OPTIONS preflight request"):
            headers = {
                "Origin": "http://busybar.local",
                "Access-Control-Request-Method": "GET",
                "Access-Control-Request-Headers": "Content-Type",
            }
            response = api.options(endpoint, headers=headers)

        with allure.step("Verify OPTIONS response is successful"):
            assert response.status_code in [200, 204], (
                f"OPTIONS {endpoint} failed with status {response.status_code}. "
                f"Expected 200 or 204 for CORS preflight."
            )

        with allure.step("Check CORS headers are present"):
            resp_headers = response.headers
            allure.attach(
                "\n".join(f"{k}: {v}" for k, v in resp_headers.items()),
                name="Response Headers",
                attachment_type=allure.attachment_type.TEXT,
            )
