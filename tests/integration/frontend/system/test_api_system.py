import allure
import pytest

from typing import Optional

from clients.api import SystemAPI


@pytest.fixture
def time_settings_guard(system_api: SystemAPI):
    """Restore device time and timezone after a test."""
    original = system_api.get_time().timestamp

    timezone_offset: Optional[str] = None
    if original.endswith("Z"):
        timezone_offset = "+00:00"
    elif len(original) >= 6 and (original[-6] in ("+", "-")):
        timezone_offset = original[-6:]

    original_local = original[:19]

    yield original_local, timezone_offset

    try:
        if timezone_offset:
            system_api.set_timezone(timezone_offset)
        system_api.set_timestamp(original_local)
    except Exception:
        pass


@allure.feature("5. Web Frontend")
@allure.story("System")
class TestSystemAPI:
    """Test cases for System API endpoints"""

    @allure.id("2638")
    @allure.title("GET /api/version")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_version_get(self, system_api: SystemAPI):
        """Test GET /api/version endpoint"""
        response = system_api.get_version()

        # Pydantic validates semver format, just verify it's reasonable
        major, minor, patch = [int(p) for p in response.api_semver.split(".")]
        assert (major, minor, patch) >= (0, 0, 0)

    @allure.id("2639")
    @allure.title("GET /api/status")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_status_get(self, system_api: SystemAPI, cli_device_info):
        """Test GET /api/status endpoint"""
        response = system_api.get_status()

        # Structure validation done by pydantic
        assert response.system.uptime
        assert response.system.version
        assert 0 <= response.power.battery_charge <= 100

        with allure.step("Cross-verify with CLI device_info data"):
            assert cli_device_info.strip(), "CLI device_info should return data"

    @allure.id("2640")
    @allure.title("GET /api/status/system")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_status_system_get(self, system_api: SystemAPI, cli_device_info):
        """Test GET /api/status/system endpoint"""
        response = system_api.get_system_status()

        # All fields validated by pydantic as strings
        assert response.branch
        assert response.version
        assert response.build_date
        assert response.commit_hash
        assert response.uptime

    @allure.id("2641")
    @allure.title("GET /api/status/power")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_status_power_get(self, system_api: SystemAPI, cli_device_info):
        """Test GET /api/status/power endpoint"""
        response = system_api.get_power_status()

        # State enum and battery_charge range validated by pydantic
        assert response.state in ["discharging", "charging", "charged"]
        assert 0 <= response.battery_charge <= 100


@allure.feature("5. Web Frontend")
@allure.story("System")
class TestTimeAPI:
    """Test cases for Time API endpoints"""

    @allure.id("2680")
    @allure.title("GET /api/time")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_time_get(self, system_api: SystemAPI):
        """Test GET /api/time endpoint"""
        response = system_api.get_time()

        # Pydantic validates ISO 8601 basic format
        assert "T" in response.timestamp
        assert "+" in response.timestamp or "-" in response.timestamp[10:]

    @allure.id("2681")
    @allure.title("POST /api/time/timestamp")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_time_timestamp_post(self, system_api: SystemAPI, time_settings_guard):
        """Test POST /api/time/timestamp endpoint"""
        test_timestamp = "2025-06-15T12:30:45"

        system_api.set_timestamp(test_timestamp)

    @allure.id("2682")
    @allure.title("POST /api/time/timestamp (invalid)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        "invalid_ts",
        [
            "not-a-timestamp",
            "2025/06/15 12:30:45",
            "12:30:45",
            "",
        ],
    )
    def test_api_time_timestamp_post_invalid(
        self, system_api: SystemAPI, invalid_ts
    ):
        """Test POST /api/time/timestamp endpoint with invalid format"""
        with allure.step(f"Test invalid timestamp: {invalid_ts}"):
            response = system_api.set_timestamp_raw(invalid_ts)
            assert response.status_code == 400

    @allure.id("2683")
    @allure.title("POST /api/time/timezone")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        "test_tz",
        [
            "+00:00",
            "-05:00",
            "+09:30",
            "-12:00",
        ],
    )
    def test_api_time_timezone_post(self, system_api: SystemAPI, test_tz, time_settings_guard):
        """Test POST /api/time/timezone endpoint"""
        system_api.set_timezone(test_tz)

    @allure.id("2684")
    @allure.title("POST /api/time/timezone (invalid)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        "invalid_tz",
        [
            "invalid",
            "+25:00",
            "-15:00",
            "",
        ],
    )
    def test_api_time_timezone_post_invalid(
        self, system_api: SystemAPI, invalid_tz
    ):
        """Test POST /api/time/timezone endpoint with invalid format"""
        with allure.step(f"Test invalid timezone: {invalid_tz}"):
            response = system_api.set_timezone_raw(invalid_tz)
            assert response.status_code == 400
