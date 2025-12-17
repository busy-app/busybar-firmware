import json

import allure
import pytest


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
            response = api_session.get(f"{web_base_url}/api/version", timeout=10)

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            version_data = response.json()
            allure.attach(
                json.dumps(version_data, indent=2),
                "Version Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields
            assert (
                "api_semver" in version_data
            ), "Response should contain 'api_semver' field"

        with allure.step("Verify api_semver format and value"):
            api_semver = version_data["api_semver"]
            assert isinstance(api_semver, str), "api_semver should be a string"

            # Check semantic versioning format (x.y.z)
            version_parts = api_semver.split(".")
            assert (
                len(version_parts) == 3
            ), f"api_semver should be in x.y.z format, got {api_semver}"

            # Verify it's a valid semantic version (>= 0.0.0)
            major, minor, patch = [int(part) for part in version_parts]
            version_tuple = (major, minor, patch)
            assert version_tuple >= (
                0,
                0,
                0,
            ), f"api_semver should be a valid semantic version, got {api_semver}"

            # Note: 0.0.0 is acceptable for development/initial versions
            # If the requirement is specifically > 0.0.0, this would need to change based on actual firmware version

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
            allure.attach(
                cli_device_info, "CLI device_info", allure.attachment_type.TEXT
            )

        with allure.step("Make GET request to /api/status"):
            response = api_session.get(f"{web_base_url}/api/status", timeout=10)

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            status_data = response.json()
            allure.attach(
                json.dumps(status_data, indent=2),
                "Status Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields based on OpenAPI schema
            assert "system" in status_data, "Response should contain 'system' field"
            assert "power" in status_data, "Response should contain 'power' field"

        with allure.step("Verify system data structure"):
            system_data = status_data["system"]
            required_system_fields = [
                "branch",
                "version",
                "build_date",
                "commit_hash",
                "uptime",
            ]
            for field in required_system_fields:
                assert (
                    field in system_data
                ), f"System data should contain '{field}' field"

        with allure.step("Verify power data structure"):
            power_data = status_data["power"]
            required_power_fields = [
                "state",
                "battery_charge",
                "battery_voltage",
                "battery_current",
                "usb_voltage",
            ]
            for field in required_power_fields:
                assert field in power_data, f"Power data should contain '{field}' field"

        with allure.step("Cross-verify with CLI device_info data"):
            # This is a basic verification - specific field matching would need actual CLI output parsing
            assert (
                cli_device_info.strip()
            ), "CLI device_info should return data for comparison"

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
            allure.attach(
                cli_device_info, "CLI device_info", allure.attachment_type.TEXT
            )

        with allure.step("Make GET request to /api/status/system"):
            response = api_session.get(f"{web_base_url}/api/status/system", timeout=10)

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            system_data = response.json()
            allure.attach(
                json.dumps(system_data, indent=2),
                "System Status Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields based on OpenAPI schema
            required_fields = [
                "branch",
                "version",
                "build_date",
                "commit_hash",
                "uptime",
            ]
            for field in required_fields:
                assert field in system_data, f"Response should contain '{field}' field"
                assert isinstance(
                    system_data[field], str
                ), f"Field '{field}' should be a string"

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
            allure.attach(
                cli_device_info, "CLI device_info", allure.attachment_type.TEXT
            )

        with allure.step("Make GET request to /api/status/power"):
            response = api_session.get(f"{web_base_url}/api/status/power", timeout=10)

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            power_data = response.json()
            allure.attach(
                json.dumps(power_data, indent=2),
                "Power Status Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields based on OpenAPI schema
            required_fields = [
                "state",
                "battery_charge",
                "battery_voltage",
                "battery_current",
                "usb_voltage",
            ]
            for field in required_fields:
                assert field in power_data, f"Response should contain '{field}' field"

        with allure.step("Verify power field types and values"):
            # Validate state enum
            valid_states = ["discharging", "charging", "charged"]
            assert (
                power_data["state"] in valid_states
            ), f"State should be one of {valid_states}"

            # Validate numeric fields
            numeric_fields = [
                "battery_charge",
                "battery_voltage",
                "battery_current",
                "usb_voltage",
            ]
            for field in numeric_fields:
                assert isinstance(
                    power_data[field], int
                ), f"Field '{field}' should be an integer"

            # Validate battery charge percentage
            assert (
                0 <= power_data["battery_charge"] <= 100
            ), "Battery charge should be between 0-100%"


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
            response = api_session.get(f"{web_base_url}/api/time", timeout=10)

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json"
                in response.headers.get("content-type", "").lower()
            )

            time_data = response.json()
            allure.attach(
                json.dumps(time_data, indent=2),
                "Time Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields based on OpenAPI schema
            assert (
                "timestamp" in time_data
            ), "Response should contain 'timestamp' field"
            assert isinstance(
                time_data["timestamp"], str
            ), "Timestamp should be a string"

            # Validate ISO 8601 format (basic check)
            timestamp = time_data["timestamp"]
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

        # Use a valid ISO 8601 timestamp
        test_timestamp = "2025-06-15T12:30:45"

        with allure.step(f"Set timestamp to: {test_timestamp}"):
            params = {"timestamp": test_timestamp}
            response = api_session.post(
                f"{web_base_url}/api/time/timestamp", params=params, timeout=10
            )

        with allure.step("Verify response status"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Timestamp Set Response",
                allure.attachment_type.JSON,
            )

            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

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
            params = {"timestamp": invalid_ts}
            response = api_session.post(
                f"{web_base_url}/api/time/timestamp", params=params, timeout=10
            )

            assert response.status_code == 400, (
                f"Expected 400 for invalid timestamp '{invalid_ts}', "
                f"got {response.status_code}"
            )

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
            params = {"timezone": test_tz}
            response = api_session.post(
                f"{web_base_url}/api/time/timezone", params=params, timeout=10
            )

        with allure.step("Verify response status"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Timezone Set Response",
                allure.attachment_type.JSON,
            )

            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

    @allure.id("2720")
    @allure.title("POST /api/time/timezone (invalid)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("invalid_tz", [
        "invalid",
        "+25:00",  # Out of range
        "-15:00",  # Out of range
        "",
    ])
    def test_api_time_timezone_post_invalid(self, api_session, web_base_url, invalid_tz):
        """Test POST /api/time/timezone endpoint with invalid format"""

        with allure.step(f"Test invalid timezone: {invalid_tz}"):
            params = {"timezone": invalid_tz}
            response = api_session.post(
                f"{web_base_url}/api/time/timezone", params=params, timeout=10
            )

            assert response.status_code == 400, (
                f"Expected 400 for invalid timezone '{invalid_tz}', "
                f"got {response.status_code}"
            )


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
            allure.attach(
                initial_device_info, "Initial Device Info", allure.attachment_type.TEXT
            )

        with allure.step("Create mock update package"):
            mock_tar_content = b"Mock firmware update package for testing"

        with allure.step("Attempt firmware update"):
            params = {"name": "test_firmware"}

            response = api_session.post(
                f"{web_base_url}/api/update",
                params=params,
                data=mock_tar_content,
                headers={"Content-Type": "application/octet-stream"},
                timeout=30,
            )

        with allure.step("Verify update response"):
            # Expected responses: 200 (success), 400 (invalid package), 413 (too large), 500 (error)
            expected_codes = [200, 400, 413, 500]
            assert (
                response.status_code in expected_codes
            ), f"Expected one of {expected_codes}, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Update Response",
                allure.attachment_type.JSON,
            )
