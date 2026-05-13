import json

import allure
import pytest

from clients.api import AccountAPI
from clients.api.account import AccountBackend


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
@pytest.mark.uses_si917
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
@pytest.mark.uses_si917
class TestAccountBackendAPI:
    """Test cases for MQTT backend configuration API endpoints (FW-881)."""

    @allure.id("3488")
    @allure.title("GET /api/account/backend")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_backend_get(self, account_api: AccountAPI):
        """Test GET /api/account/backend returns a valid AccountBackend structure."""
        response = account_api.get_backend()

        # Structure validated by pydantic
        assert response.server_url
        assert response.client_cert_type in ["default", "custom", "none"]
        assert isinstance(response.ignore_server_cert, bool)

    @allure.id("3487")
    @allure.title("PUT /api/account/backend (custom URL)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_backend_put_custom_url(self, account_api: AccountAPI):
        """Test PUT /api/account/backend with a custom server URL."""
        custom_url = "mqtts://mqtt.example.com:8883"

        with allure.step("Capture original config"):
            original = account_api.get_backend()
            allure.attach(
                json.dumps(original.model_dump(), indent=2),
                name="Original Backend Config",
                attachment_type=allure.attachment_type.JSON,
            )

        new_config = AccountBackend(
            server_url=custom_url,
            client_cert_type="default",
            ignore_server_cert=False,
        )
        account_api.set_backend(new_config)

        with allure.step("Verify config was updated"):
            verify = account_api.get_backend()
            assert verify.server_url == custom_url

        with allure.step("Restore original config"):
            account_api.set_backend(original)

    @allure.id("3835")
    @allure.title("PUT /api/account/backend (default)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_backend_put_default(self, account_api: AccountAPI):
        """Test PUT /api/account/backend with server_url='default'."""
        with allure.step("Capture original config"):
            original = account_api.get_backend()

        account_api.set_backend(
            AccountBackend(
                server_url="default",
                client_cert_type="default",
                ignore_server_cert=False,
            )
        )

        with allure.step("Verify default was set"):
            verify = account_api.get_backend()
            assert verify.server_url == "default"

        with allure.step("Restore original config"):
            account_api.set_backend(original)

    @allure.id("3836")
    @allure.title("PUT /api/account/backend (invalid)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        "payload,reason",
        [
            ({}, "missing all required fields"),
            (
                {"server_url": "default"},
                "missing client_cert_type and ignore_server_cert",
            ),
            (
                {
                    "server_url": "default",
                    "client_cert_type": "invalid",
                    "ignore_server_cert": False,
                },
                "invalid client_cert_type value",
            ),
        ],
    )
    def test_api_account_backend_put_invalid(
        self, account_api: AccountAPI, payload, reason
    ):
        """Test PUT /api/account/backend with invalid payloads returns 400."""
        response = account_api.set_backend_raw(payload)

        assert (
            response.status_code == 400
        ), f"Expected 400 for payload {payload!r} ({reason}), got {response.status_code}"


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
            response = account_api.get_status()
            allure.attach(json.dumps({"state": response.status}, indent=2), name="MQTT Status Before Link", attachment_type=allure.attachment_type.JSON)

            if response.status != "connected":
                pytest.skip(f"MQTT is not connected (state: {response.status})")

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
    def test_api_account_delete(self, account_api: AccountAPI):
        """Test DELETE /api/account endpoint (unlink)"""
        account_api.unlink()

        with allure.step("Verify account is unlinked"):
            verify = account_api.get_info()
            assert verify.linked is False
