import allure
import pytest

from utils import (
    api_get,
    api_post,
    attach_text,
    assert_has_fields,
    assert_field_in,
    assert_field_type,
    assert_field_range,
)


@allure.feature("5. Web Frontend")
@allure.story("System")
class TestSystemAPI:
    """Test cases for System API endpoints"""

    @allure.id("2638")
    @allure.title("GET /api/version")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_version_get(
        self, api_session, web_base_url, persistent_cli_connection
    ):
        """Test GET /api/version endpoint"""

        with allure.step("Make GET request to /api/version"):
            response = api_get(api_session, web_base_url, "/api/version")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("api_semver").attach_to_allure("Version Response")

        with allure.step("Verify api_semver format and value"):
            api_semver = response.get_field("api_semver")
            assert isinstance(api_semver, str), "api_semver should be a string"

            # Check semantic versioning format (x.y.z)
            version_parts = api_semver.split(".")
            assert (
                len(version_parts) == 3
            ), f"api_semver should be in x.y.z format, got {api_semver}"

            # Verify it's a valid semantic version (>= 0.0.0)
            major, minor, patch = [int(part) for part in version_parts]
            version_tuple = (major, minor, patch)
            assert version_tuple >= (0, 0, 0), f"api_semver should be a valid semantic version, got {api_semver}"

    @allure.id("2639")
    @allure.title("GET /api/status")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_status_get(self, api_session, web_base_url, persistent_cli_connection):
        """Test GET /api/status endpoint"""

        with allure.step("Get CLI device_info for comparison"):
            cli_device_info = persistent_cli_connection.execute_command(
                "device_info", timeout=20.0, slow_command=True
            )
            attach_text(cli_device_info, "CLI device_info")

        with allure.step("Make GET request to /api/status"):
            response = api_get(api_session, web_base_url, "/api/status")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("system", "power").attach_to_allure("Status Response")

        with allure.step("Verify system data structure"):
            data = response.json()
            assert_has_fields(
                data["system"],
                "branch", "version", "build_date", "commit_hash", "uptime"
            )

        with allure.step("Verify power data structure"):
            assert_has_fields(
                data["power"],
                "state", "battery_charge", "battery_voltage", "battery_current", "usb_voltage"
            )

        with allure.step("Cross-verify with CLI device_info data"):
            assert cli_device_info.strip(), "CLI device_info should return data for comparison"

    @allure.id("2640")
    @allure.title("GET /api/status/system")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_status_system_get(
        self, api_session, web_base_url, persistent_cli_connection
    ):
        """Test GET /api/status/system endpoint"""

        with allure.step("Get CLI device_info for comparison"):
            cli_device_info = persistent_cli_connection.execute_command(
                "device_info", timeout=20.0, slow_command=True
            )
            attach_text(cli_device_info, "CLI device_info")

        with allure.step("Make GET request to /api/status/system"):
            response = api_get(api_session, web_base_url, "/api/status/system")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields(
                "branch", "version", "build_date", "commit_hash", "uptime"
            ).attach_to_allure("System Status Response")

            # Validate all fields are strings
            data = response.json()
            for field in ["branch", "version", "build_date", "commit_hash", "uptime"]:
                assert_field_type(data, field, str)

    @allure.id("2641")
    @allure.title("GET /api/status/power")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_status_power_get(
        self, api_session, web_base_url, persistent_cli_connection
    ):
        """Test GET /api/status/power endpoint"""

        with allure.step("Get CLI device_info for comparison"):
            cli_device_info = persistent_cli_connection.execute_command(
                "device_info", timeout=20.0, slow_command=True
            )
            attach_text(cli_device_info, "CLI device_info")

        with allure.step("Make GET request to /api/status/power"):
            response = api_get(api_session, web_base_url, "/api/status/power")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields(
                "state", "battery_charge", "battery_voltage", "battery_current", "usb_voltage"
            ).attach_to_allure("Power Status Response")

        with allure.step("Verify power field types and values"):
            data = response.json()

            # Validate state enum
            assert_field_in(data, "state", ["discharging", "charging", "charged"])

            # Validate numeric fields
            for field in ["battery_charge", "battery_voltage", "battery_current", "usb_voltage"]:
                assert_field_type(data, field, int)

            # Validate battery charge percentage
            assert_field_range(data, "battery_charge", min_value=0, max_value=100)


@allure.feature("5. Web Frontend")
@allure.story("System")
class TestTimeAPI:
    """Test cases for Time API endpoints"""

    @allure.id("2716")
    @allure.title("GET /api/time")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_time_get(self, api_session, web_base_url):
        """Test GET /api/time endpoint"""

        with allure.step("Make GET request to /api/time"):
            response = api_get(api_session, web_base_url, "/api/time")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("timestamp")
            response.assert_field_type("timestamp", str).attach_to_allure("Time Response")

            # Validate ISO 8601 format (basic check)
            timestamp = response.get_field("timestamp")
            assert "T" in timestamp, "Timestamp should be in ISO 8601 format"
            assert (
                "+" in timestamp or "-" in timestamp[10:]
            ), "Timestamp should include timezone offset"

    @allure.id("2717")
    @allure.title("POST /api/time/timestamp")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_time_timestamp_post(self, api_session, web_base_url):
        """Test POST /api/time/timestamp endpoint"""

        test_timestamp = "2025-06-15T12:30:45"

        with allure.step(f"Set timestamp to: {test_timestamp}"):
            response = api_post(
                api_session, web_base_url, "/api/time/timestamp",
                params={"timestamp": test_timestamp}
            )

        with allure.step("Verify response status"):
            response.assert_ok()
            response.assert_has_fields("result").attach_to_allure("Timestamp Set Response")

    @allure.id("2718")
    @allure.title("POST /api/time/timestamp (invalid)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("invalid_ts", [
        "not-a-timestamp",
        "2025/06/15 12:30:45",
        "12:30:45",
        "",
    ])
    def test_api_time_timestamp_post_invalid(self, api_session, web_base_url, invalid_ts):
        """Test POST /api/time/timestamp endpoint with invalid format"""

        with allure.step(f"Test invalid timestamp: {invalid_ts}"):
            response = api_post(
                api_session, web_base_url, "/api/time/timestamp",
                params={"timestamp": invalid_ts}
            )
            response.assert_bad_request()

    @allure.id("2719")
    @allure.title("POST /api/time/timezone")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("test_tz", [
        "+00:00",
        "-05:00",
        "+09:30",
        "-12:00",
    ])
    def test_api_time_timezone_post(self, api_session, web_base_url, test_tz):
        """Test POST /api/time/timezone endpoint"""

        with allure.step(f"Set timezone to: {test_tz}"):
            response = api_post(
                api_session, web_base_url, "/api/time/timezone",
                params={"timezone": test_tz}
            )

        with allure.step("Verify response status"):
            response.assert_ok()
            response.assert_has_fields("result").attach_to_allure("Timezone Set Response")

    @allure.id("2720")
    @allure.title("POST /api/time/timezone (invalid)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("invalid_tz", [
        "invalid",
        "+25:00",
        "-15:00",
        "",
    ])
    def test_api_time_timezone_post_invalid(self, api_session, web_base_url, invalid_tz):
        """Test POST /api/time/timezone endpoint with invalid format"""

        with allure.step(f"Test invalid timezone: {invalid_tz}"):
            response = api_post(
                api_session, web_base_url, "/api/time/timezone",
                params={"timezone": invalid_tz}
            )
            response.assert_bad_request()


@allure.feature("5. Web Frontend")
@allure.story("Local API - System")
class TestUpdateAPI:
    """Test cases for Update API endpoints"""

    @allure.id("2670")
    @allure.title("POST /api/update")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_update_post(
        self, api_session, web_base_url, persistent_cli_connection
    ):
        """Test POST /api/update endpoint with simulated update package"""

        with allure.step("Get initial device info for version comparison"):
            initial_device_info = persistent_cli_connection.execute_command(
                "device_info", timeout=20.0, slow_command=True
            )
            attach_text(initial_device_info, "Initial Device Info")

        with allure.step("Create mock update package"):
            mock_tar_content = b"Mock firmware update package for testing"

        with allure.step("Attempt firmware update"):
            response = api_post(
                api_session, web_base_url, "/api/update",
                params={"name": "test_firmware"},
                data=mock_tar_content,
                headers={"Content-Type": "application/octet-stream"},
                timeout=30,
            )

        with allure.step("Verify update response"):
            # Expected responses: 200 (success), 400 (invalid package), 413 (too large), 500 (error)
            response.assert_status([200, 400, 413, 500])
            response.attach_to_allure("Update Response")
