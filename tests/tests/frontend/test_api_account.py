import json

import allure
import pytest

from api import AccountAPI


@allure.feature("5. Web Frontend")
@allure.story("Account")
class TestAccountInfoAPI:
    """Test cases for Account Info API endpoint"""

    @allure.id("3490")
    @allure.title("GET /api/account/info")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_info_get(self, account_api: AccountAPI):
        """Test GET /api/account/info endpoint"""
        response = account_api.get_info()

        # linked field validated by pydantic as bool
        assert isinstance(response.linked, bool)

        # If linked, additional fields should be present
        if response.linked:
            assert response.id
            assert response.email
            assert response.user_id


@allure.feature("5. Web Frontend")
@allure.story("Account")
class TestAccountStatusAPI:
    """Test cases for Account Status (MQTT) API endpoint"""

    @allure.id("3489")
    @allure.title("GET /api/account/status")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_status_get(self, account_api: AccountAPI):
        """Test GET /api/account/status endpoint"""
        response = account_api.get_status()

        assert response.status in ["error", "disconnected", "connected"]


@allure.feature("5. Web Frontend")
@allure.story("Account")
class TestAccountProfileAPI:
    """Test cases for Account Profile (MQTT backend) API endpoints"""

    @allure.id("3488")
    @allure.title("GET /api/account/profile")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_profile_get(self, account_api: AccountAPI):
        """Test GET /api/account/profile endpoint"""
        response = account_api.get_profile()

        assert response.profile in ["dev", "prod", "local", "custom"]

        if response.profile == "custom":
            assert response.custom_url

    @allure.id("3487")
    @allure.title("POST /api/account/profile")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("profile", ["dev", "prod"])
    def test_api_account_profile_post(self, account_api: AccountAPI, profile):
        """Test POST /api/account/profile endpoint"""
        with allure.step("Get current profile to restore later"):
            original = account_api.get_profile()
            allure.attach(
                json.dumps({"profile": original.profile, "custom_url": original.custom_url}, indent=2),
                name="Original Profile",
                attachment_type=allure.attachment_type.JSON
            )

        response = account_api.set_profile(profile)
        assert response.result

        with allure.step("Verify profile was updated"):
            verify = account_api.get_profile()
            assert verify.profile == profile

        # Restore original profile
        if original.profile != profile:
            with allure.step(f"Restore original profile: {original.profile}"):
                account_api.set_profile(original.profile, original.custom_url)

    @allure.id("3487")
    @allure.title("POST /api/account/profile (custom)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_profile_post_custom(self, account_api: AccountAPI):
        """Test POST /api/account/profile endpoint with custom URL"""
        custom_url = "mqtts://mqtt.example.com:8883"

        with allure.step("Get current profile to restore later"):
            original = account_api.get_profile()

        response = account_api.set_profile("custom", custom_url)
        assert response.result

        with allure.step("Verify custom profile was set"):
            verify = account_api.get_profile()
            assert verify.profile == "custom"
            assert verify.custom_url == custom_url

        # Restore original profile
        if original.profile:
            with allure.step(f"Restore original profile: {original.profile}"):
                account_api.set_profile(original.profile, original.custom_url)

    @allure.id("3486")
    @allure.title("POST /api/account/profile (invalid)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("profile", ["invalid", "", "production", "development"])
    def test_api_account_profile_post_invalid(self, account_api: AccountAPI, profile):
        """Test POST /api/account/profile endpoint with invalid profile"""
        response = account_api.set_profile_raw(profile)

        assert response.status_code == 400


@allure.feature("5. Web Frontend")
@allure.story("Account")
class TestAccountLinkAPI:
    """Test cases for Account Link API endpoint"""

    @allure.id("3485")
    @allure.title("POST /api/account/link")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_link_post(self, account_api: AccountAPI):
        """Test POST /api/account/link endpoint"""
        with allure.step("Check account info before linking"):
            info = account_api.get_info()
            allure.attach(json.dumps({"linked": info.linked}, indent=2), name="Account Info Before Link", attachment_type=allure.attachment_type.JSON)

            if info.linked:
                pytest.skip("Account is already linked")

        with allure.step("Check MQTT status before linking"):
            status = account_api.get_status()
            allure.attach(json.dumps({"status": status.status}, indent=2), name="MQTT Status Before Link", attachment_type=allure.attachment_type.JSON)

            if status.status != "connected":
                pytest.skip(f"MQTT is not connected (state: {status.status})")

        response = account_api.link()

        # Validated by pydantic
        assert response.code
        assert len(response.code) > 0
        assert response.expires_at > 1577836800  # After year 2020


@allure.feature("5. Web Frontend")
@allure.story("Account")
class TestAccountUnlinkAPI:
    """Test cases for Account Unlink API endpoint"""

    @allure.id("3484")
    @allure.title("DELETE /api/account")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Destructive test - unlinks account")
    def test_api_account_delete(self, account_api: AccountAPI):
        """Test DELETE /api/account endpoint (unlink)"""
        response = account_api.unlink()
        assert response.result

        with allure.step("Verify account is unlinked"):
            verify = account_api.get_info()
            assert verify.linked is False
