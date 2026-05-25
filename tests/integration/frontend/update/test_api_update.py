import json
import time
from time import sleep

import allure
import pytest
import requests

from clients.api import UpdateAPI


# Example firmware version for negative testing
ERROR_FIRMWARE_VERSION = "0.0.0"


@pytest.fixture
def autoupdate_guard(update_api: UpdateAPI):
    original = update_api.get_autoupdate()
    yield original
    payload = {
        "is_enabled": original.is_enabled,
        "interval_start": original.interval_start,
        "interval_end": original.interval_end,
    }
    with requests.Session() as session:
        session.headers.update({"Accept": "application/json"})
        deadline = time.monotonic() + 10
        while time.monotonic() < deadline:
            try:
                response = session.post(
                    f"{update_api.base_url}/api/update/autoupdate",
                    json=payload,
                    timeout=5,
                )
                if response.status_code == 200:
                    return
            except requests.RequestException:
                time.sleep(0.5)


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
class TestAutoupdateAPI:
    """Test cases for Autoupdate API endpoints"""

    @allure.title("GET /api/update/autoupdate")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_autoupdate_get(self, update_api: UpdateAPI):
        """Test GET /api/update/autoupdate returns settings"""
        response = update_api.get_autoupdate()

        assert isinstance(response.is_enabled, bool)
        assert response.interval_start
        assert response.interval_end

        # Validate time format HH:MM
        for time_str in [response.interval_start, response.interval_end]:
            parts = time_str.split(":")
            assert len(parts) == 2
            hours, minutes = int(parts[0]), int(parts[1])
            assert 0 <= hours <= 23
            assert 0 <= minutes <= 59

    @allure.title("POST /api/update/autoupdate (set and restore)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_autoupdate_set(self, update_api: UpdateAPI):
        """Test POST /api/update/autoupdate sets settings"""
        # Save original settings
        original = update_api.get_autoupdate()

        test_settings = {
            "is_enabled": not original.is_enabled,
            "interval_start": "03:00",
            "interval_end": "06:00",
        }

        try:
            with allure.step("Set test autoupdate settings"):
                update_api.set_autoupdate(test_settings)

            with allure.step("Verify settings were applied"):
                updated = update_api.get_autoupdate()
                assert updated.is_enabled == test_settings["is_enabled"]
                assert updated.interval_start == "03:00"
                assert updated.interval_end == "06:00"
        finally:
            with allure.step("Restore original settings"):
                update_api.set_autoupdate({
                    "is_enabled": original.is_enabled,
                    "interval_start": original.interval_start,
                    "interval_end": original.interval_end,
                })

    @allure.title("POST /api/update/autoupdate (invalid time)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_autoupdate_invalid_time(self, update_api: UpdateAPI):
        """Test POST /api/update/autoupdate rejects invalid time format"""
        response = update_api.set_autoupdate_raw({
            "interval_start": "invalid",
            "interval_end": "25:00",
        })
        assert response.status_code == 400

    @allure.title("POST /api/update/autoupdate supports partial updates")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.regression
    def test_autoupdate_partial_update_preserves_omitted_fields(
        self, update_api: UpdateAPI, autoupdate_guard
    ):
        update_api.set_autoupdate(
            {"is_enabled": False, "interval_start": "01:15", "interval_end": "22:45"}
        )
        update_api.set_autoupdate({"interval_start": "23:30"})

        updated = update_api.get_autoupdate()
        assert updated.is_enabled is False
        assert updated.interval_start == "23:30"
        assert updated.interval_end == "22:45"

    @allure.title("POST /api/update/autoupdate accepts boundary windows")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.regression
    def test_autoupdate_boundary_window_values(
        self, update_api: UpdateAPI, autoupdate_guard
    ):
        update_api.set_autoupdate(
            {"is_enabled": True, "interval_start": "23:59", "interval_end": "00:00"}
        )

        updated = update_api.get_autoupdate()
        assert updated.is_enabled is True
        assert updated.interval_start == "23:59"
        assert updated.interval_end == "00:00"

    @allure.title("POST /api/update/autoupdate rejects invalid time values")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.regression
    @pytest.mark.parametrize(
        "payload",
        [
            {"interval_start": "24:00"},
            {"interval_start": "-01:00"},
            {"interval_end": "10:60"},
            {"interval_end": "not-a-time"},
        ],
    )
    def test_autoupdate_invalid_time_values(self, update_api: UpdateAPI, payload):
        response = update_api.set_autoupdate_raw(payload)

        assert response.status_code == 400

    @allure.title("POST /api/update/autoupdate invalid fields do not corrupt settings")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.regression
    @pytest.mark.parametrize(
        "payload",
        [
            {"is_enabled": "true"},
            {"is_enabled": 1},
            {"interval_start": 930},
            {"interval_end": None},
        ],
    )
    def test_autoupdate_invalid_field_types_do_not_corrupt_settings(
        self, update_api: UpdateAPI, autoupdate_guard, payload
    ):
        before = update_api.get_autoupdate()
        response = update_api.set_autoupdate_raw(payload)
        assert response.status_code in {200, 400}

        after = update_api.get_autoupdate()
        assert isinstance(after.is_enabled, bool)
        assert after.interval_start
        assert after.interval_end
        if response.status_code == 400:
            assert after == before

    @allure.title("POST /api/update/autoupdate unknown fields do not change settings")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.regression
    def test_autoupdate_unknown_fields_do_not_change_settings(
        self, update_api: UpdateAPI, autoupdate_guard
    ):
        before = update_api.get_autoupdate()
        response = update_api.set_autoupdate_raw({"unexpected": "field"})
        assert response.status_code in {200, 400}

        after = update_api.get_autoupdate()
        assert after == before

    @allure.title("POST /api/update/autoupdate persists after reset")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.regression
    def test_autoupdate_settings_persist_after_reset(
        self, update_api: UpdateAPI, autoupdate_guard, device_flasher
    ):
        expected = {
            "is_enabled": True,
            "interval_start": "04:00",
            "interval_end": "05:30",
        }
        update_api.set_autoupdate(expected)
        assert device_flasher.reset_and_wait(wait_timeout=90, reset_interval=15)

        session = requests.Session()
        session.headers.update({"Accept": "application/json"})
        deadline = time.monotonic() + 10
        last_error: Exception | None = None
        while time.monotonic() < deadline:
            try:
                response = session.get(
                    f"{update_api.base_url}/api/update/autoupdate", timeout=5
                )
                if response.status_code == 200:
                    restored = response.json()
                    break
            except requests.RequestException as exc:
                last_error = exc
                time.sleep(0.5)
        else:
            raise AssertionError(f"Autoupdate settings not readable after reset: {last_error}")

        assert restored["is_enabled"] == expected["is_enabled"]
        assert restored["interval_start"] == expected["interval_start"]
        assert restored["interval_end"] == expected["interval_end"]


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
                "check.status": initial.check.status,
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
            assert response.status_code in [200, 409]  # 409 if check already in progress

        with allure.step("3. Verify check is in progress or completed"):
            status = update_api.get_status()
            attach_status_json({
                "check.event": status.check.event,
                "check.status": status.check.status,
                "check.is_checking": status.check.is_checking,
            }, "Status After Check Trigger")
            assert status.check.event in ["start", "stop"]

        with allure.step("4. Wait for check to complete"):
            check_result = update_api.wait_for_check_complete(timeout=30)
            attach_status_json({
                "event": check_result.event,
                "status": check_result.status,
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
                assert status.check.status in ["available", "not_available", "failure", "none"]
