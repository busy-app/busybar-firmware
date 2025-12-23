import allure
import pytest

from utils import api_get, api_post, attach_text


# Example firmware update URL for testing
EXAMPLE_FIRMWARE_URL = "https://update.flipperzero.one/builds/busybar-firmware/0.5.0/busybar-f21-update-0.5.0.tar"
EXAMPLE_FIRMWARE_VERSION = "0.5.0"

# Non-existent firmware URL for negative testing
ERROR_FIRMWARE_URL = "https://update.flipperzero.one/builds/busybar-firmware/0.0.0/busybar-f21-update-0.0.0.tar"
ERROR_FIRMWARE_VERSION = "0.0.0"


@allure.feature("5. Web Frontend")
@allure.story("Updater")
class TestUpdateAPI:
    """Test cases for Update API endpoints"""

    @allure.id("2670")
    @allure.title("POST /api/update (invalid package)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_post_invalid_package(self, api_session, web_base_url):
        """Test POST /api/update endpoint rejects invalid update package"""

        with allure.step("Create invalid mock update package"):
            mock_tar_content = b"Invalid firmware update package for testing"

        with allure.step("Attempt firmware update with invalid package"):
            response = api_post(
                api_session,
                web_base_url,
                "/api/update",
                data=mock_tar_content,
                headers={"Content-Type": "application/octet-stream"},
                timeout=30,
            )

        with allure.step("Verify bad request response"):
            response.assert_bad_request()
            response.attach_to_allure("Update Response")

    @allure.id("3527")
    @allure.title("GET /api/update/status")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_status_get(self, api_session, web_base_url):
        """Test GET /api/update/status endpoint returns update status"""

        with allure.step("Get update status"):
            response = api_get(api_session, web_base_url, "/api/update/status")

        with allure.step("Verify response structure"):
            response.assert_ok()
            response.assert_json_content_type()
            response.attach_to_allure("Update Status Response")

            data = response.json()

            assert "install" in data, "Response should contain 'install' section"
            install = data["install"]
            assert "is_allowed" in install, "install should contain 'is_allowed'"
            assert "event" in install, "install should contain 'event'"
            assert "action" in install, "install should contain 'action'"
            assert "status" in install, "install should contain 'status'"

            valid_events = [
                "session_start",
                "session_stop",
                "action_begin",
                "action_done",
                "detail_change",
                "action_progress",
                "none",
            ]
            assert install["event"] in valid_events, (
                f"install.event should be one of {valid_events}, got {install['event']}"
            )

            valid_actions = [
                "download",
                "sha_verification",
                "unpack",
                "prepare",
                "apply",
                "none",
            ]
            assert install["action"] in valid_actions, (
                f"install.action should be one of {valid_actions}, got {install['action']}"
            )

            valid_statuses = [
                "ok",
                "battery_low",
                "busy",
                "download_failure",
                "download_abort",
                "sha_mismatch",
                "unpack_staging_dir_failure",
                "unpack_archive_open_failure",
                "unpack_archive_unpack_failure",
                "install_manifest_not_found",
                "install_manifest_invalid",
                "install_session_config_failure",
                "install_pointer_setup_failure",
                "unknown_failure",
            ]
            assert install["status"] in valid_statuses, (
                f"install.status should be one of {valid_statuses}, got {install['status']}"
            )

            # Verify check section
            assert "check" in data, "Response should contain 'check' section"
            check = data["check"]
            assert "event" in check, "check should contain 'event'"
            assert "result" in check, "check should contain 'result'"

            # Verify check.event is valid enum
            valid_check_events = ["start", "stop", "none"]
            assert check["event"] in valid_check_events, (
                f"check.event should be one of {valid_check_events}, got {check['event']}"
            )

            # Verify check.result is valid enum
            valid_check_results = ["available", "not_available", "failure", "none"]
            assert check["result"] in valid_check_results, (
                f"check.result should be one of {valid_check_results}, got {check['result']}"
            )

    @allure.id("3525")
    @allure.title("POST /api/update/check")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_check_post(self, api_session, web_base_url):
        """Test POST /api/update/check endpoint starts update check"""

        with allure.step("Start update check"):
            response = api_post(api_session, web_base_url, "/api/update/check")

        with allure.step("Verify response"):
            response.assert_ok()
            response.assert_json_content_type()
            response.assert_has_fields("result")
            response.attach_to_allure("Update Check Response")

    @allure.id("3531")
    @allure.title("GET /api/update/changelog (missing version)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_changelog_missing_version(self, api_session, web_base_url):
        """Test GET /api/update/changelog without version parameter returns error"""

        with allure.step("Request changelog without version"):
            response = api_get(api_session, web_base_url, "/api/update/changelog")

        with allure.step("Verify bad request response"):
            response.assert_bad_request()
            response.attach_to_allure("Changelog Error Response")

    @allure.id("3528")
    @allure.title("GET /api/update/changelog (non-existent version)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_changelog_nonexistent_version(self, api_session, web_base_url):
        """Test GET /api/update/changelog with non-existent version returns error"""

        with allure.step(f"Request changelog for non-existent version {ERROR_FIRMWARE_VERSION}"):
            response = api_get(
                api_session,
                web_base_url,
                "/api/update/changelog",
                params={"version": ERROR_FIRMWARE_VERSION},
            )

        with allure.step("Verify bad request response"):
            response.assert_bad_request()
            response.attach_to_allure("Changelog Error Response")

    @allure.id("3526")
    @allure.title("POST /api/update/install (missing version)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_install_missing_version(self, api_session, web_base_url):
        """Test POST /api/update/install without version parameter returns error"""

        with allure.step("Request install without version"):
            response = api_post(api_session, web_base_url, "/api/update/install")

        with allure.step("Verify bad request response"):
            response.assert_bad_request()
            response.attach_to_allure("Install Error Response")

    @allure.id("3530")
    @allure.title("POST /api/update/install (non-existent version)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_install_nonexistent_version(self, api_session, web_base_url):
        """Test POST /api/update/install with non-existent version returns error"""

        with allure.step(f"Request install for non-existent version {ERROR_FIRMWARE_VERSION}"):
            response = api_post(
                api_session,
                web_base_url,
                "/api/update/install",
                params={"version": ERROR_FIRMWARE_VERSION},
            )

        with allure.step("Verify bad request response"):
            response.assert_bad_request()
            response.attach_to_allure("Install Error Response")

    @allure.id("3529")
    @allure.title("POST /api/update/abort_download")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_abort_download(self, api_session, web_base_url):
        """Test POST /api/update/abort_download endpoint"""

        with allure.step("Send abort download signal"):
            response = api_post(api_session, web_base_url, "/api/update/abort_download")

        with allure.step("Verify response"):
            response.assert_ok()
            response.assert_json_content_type()
            response.assert_has_fields("result")
            response.attach_to_allure("Abort Download Response")
