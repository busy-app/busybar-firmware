import json

import allure
import pytest


@allure.feature("5. Web Frontend")
@allure.story("Account")
class TestAccountAPI:
    """Test cases for Account API endpoints"""

    @allure.id("2721")
    @allure.title("GET /api/account")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_get(self, api_session, web_base_url):
        """Test GET /api/account endpoint"""

        with allure.step("Make GET request to /api/account"):
            response = api_session.get(f"{web_base_url}/api/account", timeout=10)

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            account_data = response.json()
            allure.attach(
                json.dumps(account_data, indent=2),
                "Account Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields based on OpenAPI schema
            assert "state" in account_data, "Response should contain 'state' field"

            # Validate state enum
            valid_states = ["error", "disconnected", "not_linked", "linked"]
            assert (
                account_data["state"] in valid_states
            ), f"State should be one of {valid_states}, got {account_data['state']}"

            # If linked, additional fields should be present
            if account_data["state"] == "linked":
                assert "id" in account_data, "Linked state should include 'id'"
                assert "email" in account_data, "Linked state should include 'email'"

    @allure.id("2722")
    @allure.title("POST /api/account/link")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_link_post(self, api_session, web_base_url):
        """Test POST /api/account/link endpoint"""

        with allure.step("Check account status before linking"):
            status_response = api_session.get(
                f"{web_base_url}/api/account", timeout=10
            )
            assert (
                status_response.status_code == 200
            ), f"Expected 200, got {status_response.status_code}"
            account_data = status_response.json()
            account_state = account_data.get("state", "unknown")
            allure.attach(
                json.dumps(account_data, indent=2),
                "Account Status Before Link",
                allure.attachment_type.JSON,
            )

            if account_state == "linked":
                pytest.skip("Account is already linked, skipping link test")
            if account_state in ["error", "disconnected"]:
                pytest.skip(f"Account is not ready for linking (state: {account_state}), skipping link test")

        with allure.step("Request account linking PIN"):
            response = api_session.post(f"{web_base_url}/api/account/link", timeout=10)

        with allure.step("Verify response status"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Account Link Response",
                allure.attachment_type.JSON,
            )

            assert "code" in response_data, "Response should contain 'code' field"
            assert (
                "expires_at" in response_data
            ), "Response should contain 'expires_at' field"

    @allure.id("2723")
    @allure.title("DELETE /api/account")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Destructive test - unlinks account")
    def test_api_account_delete(self, api_session, web_base_url):
        """Test DELETE /api/account endpoint (unlink)"""

        with allure.step("Unlink account"):
            response = api_session.delete(f"{web_base_url}/api/account", timeout=10)

        with allure.step("Verify response status"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Account Unlink Response",
                allure.attachment_type.JSON,
            )

            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"
