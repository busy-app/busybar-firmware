import allure
import pytest

from clients.api import SettingsAPI


pytestmark = [
    pytest.mark.bsb_metadata(
        suite="Contract",
        level="Component",
        intent="Product",
        interfaces=("Device API",),
        transports=("LAN REST",),
    ),
]


@allure.epic("BusyBar & Local Control")
@allure.feature("Device HTTP API")
@allure.story("Settings")
class TestNameAPI:
    """Test cases for Name API endpoints"""

    @allure.title("Name. GET /api/name")
    @allure.issue("https://flipper.atlassian.net/browse/FW-407")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_name_get(self, settings_api: SettingsAPI):
        """Test GET /api/name endpoint"""
        response = settings_api.get_name()

        # Type validated by pydantic
        assert isinstance(response.name, str)

    @allure.title("Name. POST /api/name")
    @allure.issue("https://flipper.atlassian.net/browse/FW-407")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("test_name", [
        "Test Device Name",
        "Another Name 123",
        "T",
        "Nah - ure_ad-en",
        "Name(Paren",
        "N!",
        "Name!",
        "Name)Paren",
        "Name- Dash",
        "Name=Equal",
        "Name+Plus",
        "Name;Semicolon",
        "Name:Colon",
        "Name,Comma",
        "Name.Period",
        "Name|Pipe",
        "Name?Question",
    ])
    def test_api_name_post(self, settings_api: SettingsAPI, test_name):
        """Test POST /api/name endpoint"""
        settings_api.set_name(test_name)

        with allure.step("Verify name was updated"):
            verify = settings_api.get_name()
            assert verify.name == test_name

    @allure.title("Name. POST /api/name (negative)")
    @allure.issue("https://flipper.atlassian.net/browse/FW-407")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        "test_name",
        [
            # Empty/whitespace names
            "",
            " ",
            "  ",
            # Too long: boundary is 20 characters
            "21 chars, just over!!",  # 21 characters
            "22 symbols length name",  # 22 characters
        ],
    )
    def test_api_name_post_invalid(self, settings_api: SettingsAPI, test_name):
        """Test POST /api/name endpoint with invalid data"""
        original_name = settings_api.get_name().name

        response = settings_api.set_name_raw(test_name)
        assert response.status_code == 400

        with allure.step("Verify name was not changed"):
            assert settings_api.get_name().name == original_name

    @allure.title("Name. POST /api/name (additional properties rejected)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_name_post_additional_properties(self, settings_api: SettingsAPI):
        """Test that POST /api/name rejects bodies with extra fields (additionalProperties: false)"""
        original_name = settings_api.get_name().name

        response = settings_api.post_raw(
            "/api/name", json={"name": "ValidName", "extra": "field"}
        )
        assert response.status_code == 400

        with allure.step("Verify name was not changed"):
            assert settings_api.get_name().name == original_name


@allure.epic("BusyBar & Local Control")
@allure.feature("Device HTTP API")
@allure.story("Settings")
class TestSettingsAPI:
    """Test cases for Settings API endpoints"""

    @allure.title("Settings. GET /api/access")
    @allure.issue("FW-406")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_access_get(self, settings_api: SettingsAPI):
        """Test GET /api/access endpoint"""
        response = settings_api.get_access()

        # Mode enum and key_valid validated by pydantic
        assert response.mode in ["disabled", "enabled", "key"]
        assert isinstance(response.key_valid, bool)

    @allure.title("Settings. POST /api/access (valid keys)")
    @allure.issue("FW-406")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        "key",
        [
            "1234",
            "12345",
            "0000",
            "9999999999",
            "1234567890",
            "abcd",
            "ABCD",
            "1a45",
            "12345678901",
            pytest.param("aB3" * 10 + "Z", id="31-characters"),
            pytest.param("aB3" * 10 + "Z9", id="32-characters"),
        ],
    )
    def test_api_access_post_valid_key(self, settings_api: SettingsAPI, key):
        """Accept 4-32 ASCII letters/digits and authenticate with the full key."""
        settings_api.set_access("key", key)

        with allure.step("Verify the key is configured and authenticates requests"):
            verify = settings_api.get_access()
            assert verify.mode == "key", f"Unexpected access mode: {verify}"
            assert verify.key_valid, f"Key was not accepted: {verify}"
            response = settings_api.get_raw("/api/status", headers={"X-API-Token": key})
            assert response.status_code == 200, (
                f"Expected valid key to return HTTP 200, got "
                f"{response.status_code}: {response.text!r}"
            )

    @allure.title("Settings. POST /api/access (invalid keys)")
    @allure.issue("FW-406")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize(
        "key",
        [
            "",
            "123",
            "Ab3",
            pytest.param("A" * 33, id="33-letters"),
            pytest.param("1" * 33, id="33-digits"),
            "     ",
            "ab cd",
            "abcd!",
            "abcd_",
            "abc-9",
            "abc.9",
            "abc+9",
            "abc/9",
            pytest.param("A" * 31 + "!", id="invalid-last-character"),
            pytest.param("абвг", id="cyrillic"),
            pytest.param("éabc", id="accented-letter"),
            "ab\ncd",
            "ab\tcd",
        ],
    )
    def test_api_access_post_invalid_key(self, settings_api: SettingsAPI, key):
        """Reject invalid keys without changing the current access credential."""
        original_key = "PriorKey42"
        settings_api.set_access("key", original_key)
        original_access = settings_api.get_access()

        with allure.step("Attempt to configure an invalid key"):
            response = settings_api.set_access_raw("key", key)
            assert response.status_code == 400, (
                f"Expected invalid key to return HTTP 400, got "
                f"{response.status_code}: {response.text!r}"
            )

        with allure.step("Verify the existing access key remains active"):
            actual = settings_api.get_access()
            assert actual == original_access, f"Access settings changed: {actual}"
            response = settings_api.get_raw(
                "/api/status", headers={"X-API-Token": original_key}
            )
            assert response.status_code == 200, (
                f"Original key stopped authenticating: "
                f"HTTP {response.status_code}, body={response.text!r}"
            )

    @allure.title("Settings. GET /api/display/brightness")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_display_brightness_get(self, settings_api: SettingsAPI):
        """Test GET /api/display/brightness endpoint"""
        response = settings_api.get_brightness()

        # Structure validated by pydantic
        assert response.value is not None

    @allure.title("Settings. POST /api/display/brightness")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_display_brightness_post(self, settings_api: SettingsAPI):
        """Test POST /api/display/brightness endpoint"""
        settings_api.set_brightness(value="50")

    @allure.title("Settings. GET /api/audio/volume")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_audio_volume_get(self, settings_api: SettingsAPI):
        """Test GET /api/audio/volume endpoint"""
        response = settings_api.get_volume()

        # Type validated by pydantic
        assert isinstance(response.volume, (int, float))

    @allure.title("Settings. POST /api/audio/volume")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_audio_volume_post(self, settings_api: SettingsAPI):
        """Test POST /api/audio/volume endpoint"""
        settings_api.set_volume(50)
