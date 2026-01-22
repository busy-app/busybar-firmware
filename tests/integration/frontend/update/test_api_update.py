import json
import time
from time import sleep

import allure
import pytest

from clients.api import UpdateAPI


# Example firmware version for negative testing
ERROR_FIRMWARE_VERSION = "0.0.0"


def attach_status_json(data: dict, name: str):
    """Attach status dict as JSON to Allure report."""
    allure.attach(
        json.dumps(data, indent=2),
        name=name,
        attachment_type=allure.attachment_type.JSON
    )

@allure.feature("5. Web Frontend")
@allure.story("Updater")
class TestUpdateAPI:
    """Test cases for Update API endpoints"""

    @allure.id("2670")
    @allure.title("POST /api/update (invalid package)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_post_invalid_package(self, update_api: UpdateAPI):
        """Test POST /api/update endpoint rejects invalid update package"""
        mock_tar_content = b"Invalid firmware update package for testing"

        response = update_api.upload_package(mock_tar_content, timeout=30)

        assert response.status_code == 400

    @allure.id("3527")
    @allure.title("GET /api/update/status")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_status_get(self, update_api: UpdateAPI):
        """Test GET /api/update/status endpoint returns update status"""
        status = update_api.get_status()

        # Verify install section - idle and in_progress should be mutually exclusive
        with allure.step("Verify install status properties"):
            assert not (status.install.is_idle and status.install.is_in_progress)

        # Verify check section - at most one terminal state should be true
        with allure.step("Verify check status properties"):
            check_states = [
                status.check.is_available,
                status.check.is_up_to_date,
                status.check.has_failed,
            ]
            assert sum(check_states) <= 1

    @allure.id("3525")
    @allure.title("POST /api/update/check")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_check_post(self, update_api: UpdateAPI):
        """Test POST /api/update/check endpoint starts update check"""
        update_api.check()

    @allure.id("3531")
    @allure.title("GET /api/update/changelog (missing version)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_changelog_missing_version(self, update_api: UpdateAPI):
        """Test GET /api/update/changelog without version parameter returns error"""
        response = update_api.get_changelog_raw()

        assert response.status_code == 400

    @allure.id("3528")
    @allure.title("GET /api/update/changelog (non-existent version)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_changelog_nonexistent_version(self, update_api: UpdateAPI):
        """Test GET /api/update/changelog with non-existent version returns error"""
        response = update_api.get_changelog(ERROR_FIRMWARE_VERSION)

        assert response.status_code == 400

    @allure.id("3526")
    @allure.title("POST /api/update/install (missing version)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_install_missing_version(self, update_api: UpdateAPI):
        """Test POST /api/update/install without version parameter returns error"""
        response = update_api.install_raw()

        assert response.status_code == 400

    @allure.id("3530")
    @allure.title("POST /api/update/install (non-existent version)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_install_nonexistent_version(self, update_api: UpdateAPI):
        """Test POST /api/update/install with non-existent version returns error"""
        response = update_api.install(ERROR_FIRMWARE_VERSION)

        assert response.status_code == 400

    @allure.id("3529")
    @allure.title("POST /api/update/abort_download")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_abort_download(self, update_api: UpdateAPI):
        """Test POST /api/update/abort_download endpoint"""
        update_api.abort_download()


@allure.feature("5. Web Frontend")
@allure.story("Updater")
class TestUpdateStatusFlow:
    """Test update status transitions by triggering actions and verifying states"""

    @allure.id("3540")
    @allure.title("Update check status flow")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_update_check_status_flow(self, update_api: UpdateAPI):
        """Test status transitions during update check"""

        with allure.step("1. Get initial status"):
            initial = update_api.get_status()
            attach_status_json({
                "check.event": initial.check.event,
                "check.result": initial.check.result,
                "check.is_checking": initial.check.is_checking,
                "check.is_available": initial.check.is_available,
                "check.available_version": initial.check.available_version,
            }, "Initial Status")

        with allure.step("2. Trigger update check"):
            response = update_api.check_raw()
            allure.attach(
                f"Status code: {response.status_code}\nResponse: {response.text}",
                name="Check Response",
                attachment_type=allure.attachment_type.TEXT
            )
            assert response.status_code == 200

        with allure.step("3. Verify check is in progress or completed"):
            status = update_api.get_status()
            attach_status_json({
                "check.event": status.check.event,
                "check.result": status.check.result,
                "check.is_checking": status.check.is_checking,
            }, "Status After Check Trigger")
            assert status.check.event in ["start", "stop"]

        with allure.step("4. Wait for check to complete"):
            check_result = update_api.wait_for_check_complete(timeout=30)
            attach_status_json({
                "event": check_result.event,
                "result": check_result.result,
                "available_version": check_result.available_version,
                "is_available": check_result.is_available,
                "is_up_to_date": check_result.is_up_to_date,
                "has_failed": check_result.has_failed,
            }, "Check Result")
            assert check_result.is_available or check_result.is_up_to_date or check_result.has_failed

    @allure.id("3541")
    @allure.title("Update install and abort status flow")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_update_install_abort_flow(self, update_api: UpdateAPI):
        """Test status transitions during install and abort"""

        with allure.step("1. Check if update is available"):
            status = update_api.get_status()
            attach_status_json({
                "check.is_available": status.check.is_available,
                "check.available_version": status.check.available_version,
            }, "Current Status")

            if not status.check.is_available:
                # Try to trigger a check
                response = update_api.check()
                check_result = update_api.wait_for_check_complete(timeout=30)
                if not check_result.is_available:
                    pytest.skip("No update available to test install flow")
                version = check_result.available_version
            else:
                version = status.check.available_version

        with allure.step("2. Start install"):
            install_response = update_api.install(version)
            allure.attach(
                f"Install triggered for version: {version}\nStatus code: {install_response.status_code}",
                name="Install Trigger",
                attachment_type=allure.attachment_type.TEXT
            )

        with allure.step("3. Verify install/download started"):
            sleep(0.5)  # Brief wait for status to update
            status = update_api.get_status()
            attach_status_json({
                "install.event": status.install.event,
                "install.action": status.install.action,
                "install.status": status.install.status,
                "install.is_in_progress": status.install.is_in_progress,
                "install.is_downloading": status.install.is_downloading,
            }, "Status After Install Trigger")
            # Install should be in progress (downloading or other action)
            assert status.install.is_in_progress or status.install.event == "session_start"

        with allure.step("4. Abort download"):
            update_api.abort_download()

        with allure.step("5. Wait for abort to complete"):
            # Poll for abort to complete (may take a few seconds)
            timeout = 10
            start = time.time()
            status = None
            while time.time() - start < timeout:
                status = update_api.get_status()
                if status.install.is_idle or status.install.status == "download_abort":
                    break
                sleep(0.5)

            attach_status_json({
                "install.event": status.install.event,
                "install.action": status.install.action,
                "install.status": status.install.status,
                "install.is_idle": status.install.is_idle,
                "install.is_failed": status.install.is_failed,
                "install.failure_reason": status.install.failure_reason,
            }, "Status After Abort")
            # After abort, should be stopped or show abort status
            assert status.install.is_idle or status.install.status == "download_abort"

    @allure.id("3542")
    @allure.title("Verify idle state")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_verify_idle_state(self, update_api: UpdateAPI):
        """Test that updater is in idle state"""

        with allure.step("1. Get current status"):
            status = update_api.get_status()
            attach_status_json({
                "install.event": status.install.event,
                "install.action": status.install.action,
                "install.status": status.install.status,
                "install.is_idle": status.install.is_idle,
                "install.is_in_progress": status.install.is_in_progress,
                "check.is_checking": status.check.is_checking,
            }, "Current Status")

        with allure.step("2. Verify idle state properties"):
            # Verify consistency of idle state
            if status.install.is_idle:
                assert not status.install.is_in_progress, "idle and in_progress should be mutually exclusive"

            # If not checking, check result should be definitive
            if not status.check.is_checking:
                # status should be one of the terminal states
                assert status.check.result in ["available", "not_available", "failure", "none"]
