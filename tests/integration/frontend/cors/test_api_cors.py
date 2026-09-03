import allure
import pytest
import yaml

from clients.api import BaseAPI

_HTTP_METHODS = {"GET", "POST", "PUT", "DELETE", "PATCH"}


API_ENDPOINTS = [
    ("/api/access", "GET"),
    ("/api/access", "POST"),
    ("/api/access/tokens", "GET"),
    ("/api/access/tokens", "POST"),
    ("/api/access/tokens", "DELETE"),
    ("/api/access/tokens/{short_id}", "DELETE"),
    ("/api/account", "DELETE"),
    ("/api/account/backend", "GET"),
    ("/api/account/backend", "PUT"),
    ("/api/account/info", "GET"),
    ("/api/account/link", "POST"),
    ("/api/account/status", "GET"),
    ("/api/assets/upload", "POST"),
    ("/api/assets/upload", "DELETE"),
    ("/api/audio/play", "POST"),
    ("/api/audio/play", "DELETE"),
    ("/api/audio/volume", "GET"),
    ("/api/audio/volume", "POST"),
    ("/api/ble/disable", "POST"),
    ("/api/ble/enable", "POST"),
    ("/api/ble/pairing", "DELETE"),
    ("/api/ble/status", "GET"),
    ("/api/busy/profiles/busy", "GET"),
    ("/api/busy/profiles/busy", "PUT"),
    ("/api/busy/profiles/custom", "GET"),
    ("/api/busy/profiles/custom", "PUT"),
    ("/api/busy/snapshot", "GET"),
    ("/api/busy/snapshot", "PUT"),
    ("/api/display/brightness", "GET"),
    ("/api/display/brightness", "POST"),
    ("/api/display/draw", "POST"),
    ("/api/display/draw", "DELETE"),
    ("/api/input", "POST"),
    ("/api/log_dump", "POST"),
    ("/api/name", "GET"),
    ("/api/name", "POST"),
    ("/api/screen", "GET"),
    ("/api/smart_home/pairing", "GET"),
    ("/api/smart_home/pairing", "POST"),
    ("/api/smart_home/pairing", "DELETE"),
    ("/api/smart_home/switch", "GET"),
    ("/api/smart_home/switch", "POST"),
    ("/api/status", "GET"),
    ("/api/status/device", "GET"),
    ("/api/status/firmware", "GET"),
    ("/api/status/power", "GET"),
    ("/api/status/system", "GET"),
    ("/api/status/ws", "GET"),
    ("/api/storage/list", "GET"),
    ("/api/storage/mkdir", "POST"),
    ("/api/storage/read", "GET"),
    ("/api/storage/remove", "DELETE"),
    ("/api/storage/rename", "POST"),
    ("/api/storage/status", "GET"),
    ("/api/storage/write", "POST"),
    ("/api/time", "GET"),
    ("/api/time/timestamp", "POST"),
    ("/api/time/timezone", "GET"),
    ("/api/time/timezone", "POST"),
    ("/api/time/tzlist", "GET"),
    ("/api/transport", "GET"),
    ("/api/update", "POST"),
    ("/api/update/abort_download", "POST"),
    ("/api/update/autoupdate", "GET"),
    ("/api/update/autoupdate", "POST"),
    ("/api/update/changelog", "GET"),
    ("/api/update/check", "POST"),
    ("/api/update/install", "POST"),
    ("/api/update/status", "GET"),
    ("/api/version", "GET"),
    ("/api/wifi/connect", "POST"),
    ("/api/wifi/disconnect", "POST"),
    ("/api/wifi/networks", "GET"),
    ("/api/wifi/status", "GET"),
]


def _assert_cors_preflight(response, origin, method, requested_headers=None):
    """Assert OPTIONS preflight response satisfies CORS for the requested call."""
    assert response.status_code in (200, 204), (
        f"OPTIONS preflight failed with status {response.status_code}. "
        f"Expected 200 or 204."
    )

    allow_origin = response.headers.get("Access-Control-Allow-Origin")
    assert allow_origin in ("*", origin), (
        f"Access-Control-Allow-Origin missing or mismatched: {allow_origin!r} "
        f"(expected '*' or {origin!r})"
    )

    allow_methods = response.headers.get("Access-Control-Allow-Methods", "")
    allowed_methods = {m.strip().upper() for m in allow_methods.split(",") if m.strip()}
    assert allow_methods == "*" or method.upper() in allowed_methods, (
        f"Access-Control-Allow-Methods does not include {method}: {allow_methods!r}"
    )

    if requested_headers:
        allow_headers = response.headers.get("Access-Control-Allow-Headers", "")
        allowed_h = {h.strip().lower() for h in allow_headers.split(",") if h.strip()}
        for h in requested_headers:
            assert allow_headers == "*" or h.lower() in allowed_h, (
                f"Access-Control-Allow-Headers does not include {h!r}: "
                f"{allow_headers!r}"
            )


@allure.feature("5. Web Frontend")
@allure.story("CORS")
class TestAPICors:
    """Test cases for CORS (Cross-Origin Resource Sharing) support"""

    @allure.id("3833")
    @allure.title("OPTIONS preflight requests")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        "endpoint,method",
        API_ENDPOINTS,
        ids=[f"{m} {p}" for p, m in API_ENDPOINTS],
    )
    def test_cors_preflight_options(
        self, api_session, web_base_url, endpoint, method
    ):
        """
        Verify that the server answers OPTIONS preflight for every documented
        (endpoint, method) pair with status 200/204 and CORS headers that
        permit the requested method and Content-Type header from the
        frontend origin.
        """
        api = BaseAPI(api_session, web_base_url)
        origin = "http://10.0.4.20"

        with allure.step(f"Send OPTIONS preflight for {method} {endpoint}"):
            headers = {
                "Origin": origin,
                "Access-Control-Request-Method": method,
                "Access-Control-Request-Headers": "Content-Type",
            }
            response = api.options(endpoint, headers=headers)

            allure.attach(
                "\n".join(f"{k}: {v}" for k, v in response.headers.items()),
                name="Response Headers",
                attachment_type=allure.attachment_type.TEXT,
            )

        with allure.step("Verify CORS preflight headers"):
            _assert_cors_preflight(
                response, origin, method, requested_headers=["Content-Type"]
            )

    @allure.id("3834")
    @allure.title("OPTIONS /api/name (CORS preflight)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_cors_preflight_api_name(self, api_session, web_base_url):
        """
        Regression test for the bug where OPTIONS /api/name failed.

        Uses POST (non-simple method) to force a real preflight scenario.
        """
        api = BaseAPI(api_session, web_base_url)
        endpoint = "/api/name"
        origin = "http://busybar.local"

        with allure.step("Send OPTIONS preflight request"):
            headers = {
                "Origin": origin,
                "Access-Control-Request-Method": "POST",
                "Access-Control-Request-Headers": "Content-Type",
            }
            response = api.options(endpoint, headers=headers)

            allure.attach(
                "\n".join(f"{k}: {v}" for k, v in response.headers.items()),
                name="Response Headers",
                attachment_type=allure.attachment_type.TEXT,
            )

        with allure.step("Verify CORS preflight headers"):
            _assert_cors_preflight(
                response, origin, "POST", requested_headers=["Content-Type"]
            )

    @allure.id("3835")
    @allure.title("API_ENDPOINTS matches live /openapi.yaml")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_endpoint_list_matches_openapi(self, api_session, web_base_url):
        """
        Detect drift between the hardcoded API_ENDPOINTS list and the
        OpenAPI spec served by the device.

        Parameterized paths (e.g. /api/busy/profiles/{slot}) are checked
        by prefix — API_ENDPOINTS must contain at least one concrete
        substitution per templated path.
        """
        with allure.step("Fetch /openapi.yaml from device"):
            response = api_session.get(f"{web_base_url}/openapi.yaml", timeout=10)
            response.raise_for_status()
            spec = yaml.safe_load(response.text)

        spec_concrete = set()
        spec_templated = set()
        for path, ops in spec.get("paths", {}).items():
            for method in ops:
                upper = method.upper()
                if upper not in _HTTP_METHODS:
                    continue
                if "{" in path:
                    spec_templated.add((path, upper))
                else:
                    spec_concrete.add((path, upper))

        templated_prefixes = tuple(p.split("{", 1)[0] for p, _ in spec_templated)
        test_concrete = {
            (p, m)
            for p, m in API_ENDPOINTS
            if "{" not in p and not p.startswith(templated_prefixes)
        }

        with allure.step("Diff concrete endpoints"):
            missing = spec_concrete - test_concrete
            extra = test_concrete - spec_concrete
            assert not missing and not extra, (
                f"Drift between API_ENDPOINTS and /openapi.yaml:\n"
                f"  missing in test (add to API_ENDPOINTS): {sorted(missing)}\n"
                f"  no longer in spec (remove from API_ENDPOINTS): {sorted(extra)}"
            )

        with allure.step("Verify templated paths have concrete substitutions"):
            uncovered = []
            for tpl_path, tpl_method in spec_templated:
                prefix = tpl_path.split("{", 1)[0]
                if not any(
                    p.startswith(prefix) and m == tpl_method for p, m in API_ENDPOINTS
                ):
                    uncovered.append((tpl_path, tpl_method))
            assert not uncovered, (
                f"Templated paths from /openapi.yaml without any concrete "
                f"substitution in API_ENDPOINTS: {uncovered}"
            )
