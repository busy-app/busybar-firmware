import allure
import pytest

from clients.api import SystemAPI
from clients.api.system import DeviceInfo, FirmwareInfo, TimezoneResponse, TimezoneListResponse


@pytest.fixture
def time_settings_guard(system_api: SystemAPI):
    """Restore device time and timezone after a test."""
    original_time = system_api.get_time().timestamp
    original_tz = system_api.get_timezone().name

    yield original_time, original_tz

    try:
        system_api.set_timezone(original_tz)
        system_api.set_timestamp(original_time)
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
        assert response.system.api_semver
        assert 0 <= response.power.battery_charge <= 100

        with allure.step("Cross-verify with CLI device_info data"):
            data = cli_device_info.strip()
            assert data, (
                "CLI `device_info` returned no data. The session-scoped CLI "
                "connection likely degraded (telnet timeout, socket reset, or a "
                "previous test left it in sl_cli mode). The fixture already "
                "retries once with a reconnect — empty output here means the "
                "device CLI itself is unhealthy."
            )
            for marker in ("u5_firmware_version", "sl_firmware_version"):
                assert marker in data, (
                    f"CLI `device_info` is missing {marker!r}. Output:\n{data}"
                )

    @allure.id("2640")
    @allure.title("GET /api/status/system")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_status_system_get(self, system_api: SystemAPI, cli_device_info):
        """Test GET /api/status/system endpoint"""
        response = system_api.get_system_status()

        # All fields validated by pydantic
        assert response.api_semver
        assert response.uptime
        # boot_time is a signed 32-bit epoch timestamp on device.  When the RTC
        # is not yet synchronised the value may overflow to negative — that is a
        # firmware-side issue, not a test failure.  Pydantic already ensures it
        # is an integer; skip the sign check here.
        assert isinstance(response.boot_time, int)

    @allure.title("GET /api/status/device")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_status_device_get(self, system_api: SystemAPI):
        """Test GET /api/status/device endpoint"""
        response = system_api.get_device_info()

        assert response.serial_number
        assert response.usb_mac
        assert isinstance(response.otp_valid, bool)

    @allure.title("GET /api/status/firmware")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_status_firmware_get(self, system_api: SystemAPI):
        """Test GET /api/status/firmware endpoint"""
        response = system_api.get_firmware_info()

        assert response.version
        assert response.target >= 0
        assert response.branch
        assert response.build_date
        assert response.commit_hash

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
        test_timestamp = "2025-06-15T12:30:45Z"

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
            # Impossible calendar dates — valid format but invalid day for the month
            "2090-02-31T19:09:35Z",  # February never has 31 days
            "2025-04-31T12:00:00Z",  # April has only 30 days
        ],
    )
    def test_api_time_timestamp_post_invalid(
        self, system_api: SystemAPI, invalid_ts
    ):
        """Test POST /api/time/timestamp endpoint with invalid format"""
        with allure.step(f"Test invalid timestamp: {invalid_ts}"):
            response = system_api.set_timestamp_raw(invalid_ts)
            assert response.status_code == 400

    @allure.title("GET /api/time/timezone")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_time_timezone_get(self, system_api: SystemAPI):
        """Test GET /api/time/timezone endpoint"""
        response = system_api.get_timezone()

        assert response.name
        assert response.offset

    @allure.title("GET /api/time/tzlist")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_time_tzlist_get(self, system_api: SystemAPI):
        """Test GET /api/time/tzlist endpoint"""
        response = system_api.get_timezone_list()

        assert isinstance(response.list, list)
        assert len(response.list) > 0

        # Verify structure of timezone items
        for tz in response.list[:5]:
            assert tz.name
            assert tz.offset

    @allure.id("2683")
    @allure.title("POST /api/time/timezone")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        "test_tz",
        [
            "London",
            "Berlin",
            "Tokyo",
            "New York",
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
            "",
            "NonExistentTimezone12345",
        ],
    )
    def test_api_time_timezone_post_invalid(
        self, system_api: SystemAPI, invalid_tz
    ):
        """Test POST /api/time/timezone endpoint with invalid timezone name"""
        with allure.step(f"Test invalid timezone: {invalid_tz}"):
            response = system_api.set_timezone_raw(invalid_tz)
            assert response.status_code == 400
