import allure
import pytest

from utils import (
    api_get,
    api_post,
    api_delete,
    attach_json,
    assert_field_in,
    assert_field_type,
)


@allure.feature("5. Web Frontend")
@allure.story("Account")
class TestAccountInfoAPI:
    """Test cases for Account Info API endpoint"""

    @allure.id("3490")
    @allure.title("GET /api/account/info")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_info_get(self, api_session, web_base_url):
        """Test GET /api/account/info endpoint"""

        with allure.step("Make GET request to /api/account/info"):
            response = api_get(api_session, web_base_url, "/api/account/info")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("linked").attach_to_allure("Account Info Response")

            data = response.json()
            assert_field_type(data, "linked", bool)

            # If linked, additional fields should be present
            if data["linked"]:
                response.assert_has_fields("id", "email", "user_id")
                assert_field_type(data, "id", str)
                assert_field_type(data, "email", str)
                assert_field_type(data, "user_id", str)


@allure.feature("5. Web Frontend")
@allure.story("Account")
class TestAccountStatusAPI:
    """Test cases for Account Status (MQTT) API endpoint"""

    @allure.id("3489")
    @allure.title("GET /api/account/status")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_status_get(self, api_session, web_base_url):
        """Test GET /api/account/status endpoint"""

        with allure.step("Make GET request to /api/account/status"):
            response = api_get(api_session, web_base_url, "/api/account/status")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("status").attach_to_allure("Account Status Response")

            # Validate state enum per OpenAPI spec
            data = response.json()
            assert_field_in(data, "status", ["error", "disconnected", "connected"])


@allure.feature("5. Web Frontend")
@allure.story("Account")
class TestAccountProfileAPI:
    """Test cases for Account Profile (MQTT backend) API endpoints"""

    @allure.id("3488")
    @allure.title("GET /api/account/profile")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_profile_get(self, api_session, web_base_url):
        """Test GET /api/account/profile endpoint"""

        with allure.step("Make GET request to /api/account/profile"):
            response = api_get(api_session, web_base_url, "/api/account/profile")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("state").attach_to_allure("Account Profile Response")

            # Validate profile enum per OpenAPI spec
            data = response.json()
            assert_field_in(data, "state", ["dev", "prod", "local", "custom"])

            # If custom, should have custom_url
            if data["state"] == "custom":
                response.assert_has_fields("custom_url")

    @allure.id("3487")
    @allure.title("POST /api/account/profile")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("profile", ["dev", "prod"])
    def test_api_account_profile_post(self, api_session, web_base_url, profile):
        """Test POST /api/account/profile endpoint"""

        with allure.step("Get current profile to restore later"):
            get_response = api_get(api_session, web_base_url, "/api/account/profile")
            original_profile = None
            original_custom_url = None
            if get_response.status_code == 200:
                original_data = get_response.json()
                original_profile = original_data.get("state")
                original_custom_url = original_data.get("custom_url")
                attach_json(original_data, "Original Profile")

        with allure.step(f"Set profile to: {profile}"):
            response = api_post(
                api_session, web_base_url, "/api/account/profile",
                params={"profile": profile}
            )

        with allure.step("Verify response status"):
            response.assert_ok()
            response.assert_has_fields("result").attach_to_allure("Profile Set Response")

        with allure.step("Verify profile was updated"):
            verify_response = api_get(api_session, web_base_url, "/api/account/profile")
            verify_response.assert_ok()
            verify_data = verify_response.json()
            assert verify_data["state"] == profile, f"Expected profile '{profile}', got '{verify_data['state']}'"

        # Restore original profile
        if original_profile and original_profile != profile:
            with allure.step(f"Restore original profile: {original_profile}"):
                params = {"profile": original_profile}
                if original_profile == "custom" and original_custom_url:
                    params["custom_url"] = original_custom_url
                api_post(api_session, web_base_url, "/api/account/profile", params=params)

    @allure.id("3487")
    @allure.title("POST /api/account/profile (custom)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_profile_post_custom(self, api_session, web_base_url):
        """Test POST /api/account/profile endpoint with custom URL"""

        custom_url = "mqtts://mqtt.example.com:8883"

        with allure.step("Get current profile to restore later"):
            get_response = api_get(api_session, web_base_url, "/api/account/profile")
            original_profile = None
            original_custom_url = None
            if get_response.status_code == 200:
                original_data = get_response.json()
                original_profile = original_data.get("state")
                original_custom_url = original_data.get("custom_url")

        with allure.step(f"Set custom profile with URL: {custom_url}"):
            response = api_post(
                api_session, web_base_url, "/api/account/profile",
                params={"profile": "custom", "custom_url": custom_url}
            )

        with allure.step("Verify response status"):
            response.assert_ok()
            response.assert_has_fields("result").attach_to_allure("Custom Profile Set Response")

        with allure.step("Verify custom profile was set"):
            verify_response = api_get(api_session, web_base_url, "/api/account/profile")
            verify_response.assert_ok()
            verify_data = verify_response.json()
            assert verify_data["state"] == "custom", f"Expected profile 'custom', got '{verify_data['state']}'"
            assert verify_data.get("custom_url") == custom_url, f"Expected custom_url '{custom_url}'"

        # Restore original profile
        if original_profile:
            with allure.step(f"Restore original profile: {original_profile}"):
                params = {"profile": original_profile}
                if original_profile == "custom" and original_custom_url:
                    params["custom_url"] = original_custom_url
                api_post(api_session, web_base_url, "/api/account/profile", params=params)

    @allure.id("3486")
    @allure.title("POST /api/account/profile (invalid)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("profile", ["invalid", "", "production", "development"])
    def test_api_account_profile_post_invalid(self, api_session, web_base_url, profile):
        """Test POST /api/account/profile endpoint with invalid profile"""

        with allure.step(f"Set invalid profile: {profile!r}"):
            response = api_post(
                api_session, web_base_url, "/api/account/profile",
                params={"profile": profile}
            )

        with allure.step("Verify error response"):
            response.assert_bad_request()
            response.attach_to_allure("Invalid Profile Error Response")


@allure.feature("5. Web Frontend")
@allure.story("Account")
class TestAccountLinkAPI:
    """Test cases for Account Link API endpoint"""

    @allure.id("3485")
    @allure.title("POST /api/account/link")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_link_post(self, api_session, web_base_url):
        """Test POST /api/account/link endpoint"""

        with allure.step("Check account info before linking"):
            info_response = api_get(api_session, web_base_url, "/api/account/info")
            info_response.assert_ok()

            info_data = info_response.json()
            attach_json(info_data, "Account Info Before Link")

            if info_data.get("linked"):
                pytest.skip("Account is already linked, skipping link test")

        with allure.step("Check MQTT status before linking"):
            status_response = api_get(api_session, web_base_url, "/api/account/status")
            status_response.assert_ok()

            status_data = status_response.json()
            mqtt_state = status_data.get("status", "unknown")
            attach_json(status_data, "MQTT Status Before Link")

            if mqtt_state != "connected":
                pytest.skip(f"MQTT is not connected (state: {mqtt_state}), skipping link test")

        with allure.step("Request account linking PIN"):
            response = api_post(api_session, web_base_url, "/api/account/link")

        with allure.step("Verify response status and structure"):
            response.assert_ok()
            response.assert_has_fields("code", "expires_at").attach_to_allure("Account Link Response")

            data = response.json()
            assert_field_type(data, "code", str)
            assert_field_type(data, "expires_at", int)

            # Code should be non-empty
            assert len(data["code"]) > 0, "Link code should not be empty"

            # expires_at should be a reasonable Unix timestamp (after year 2020)
            assert data["expires_at"] > 1577836800, "expires_at should be a valid Unix timestamp"


@allure.feature("5. Web Frontend")
@allure.story("Account")
class TestAccountUnlinkAPI:
    """Test cases for Account Unlink API endpoint"""

    @allure.id("3484")
    @allure.title("DELETE /api/account")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Destructive test - unlinks account")
    def test_api_account_delete(self, api_session, web_base_url):
        """Test DELETE /api/account endpoint (unlink)"""

        with allure.step("Unlink account"):
            response = api_delete(api_session, web_base_url, "/api/account")

        with allure.step("Verify response status"):
            response.assert_ok()
            response.assert_has_fields("result").attach_to_allure("Account Unlink Response")

        with allure.step("Verify account is unlinked"):
            verify_response = api_get(api_session, web_base_url, "/api/account/info")
            verify_response.assert_ok()
            verify_data = verify_response.json()
            assert verify_data["linked"] is False, "Account should be unlinked"
