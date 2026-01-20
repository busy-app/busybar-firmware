import json

import allure
import pytest

from clients.api import SettingsAPI


@allure.feature("5. Web Frontend")
@allure.story("Settings")
class TestNameAPI:
    """Test cases for Name API endpoints"""

    @allure.id("2883")
    @allure.title("Name. GET /api/name")
    @allure.issue("https://flipper.atlassian.net/browse/FW-407")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_name_get(self, settings_api: SettingsAPI):
        """Test GET /api/name endpoint"""
        response = settings_api.get_name()

        # Type validated by pydantic
        assert isinstance(response.name, str)

    @allure.id("2884")
    @allure.title("Name. POST /api/name")
    @allure.issue("https://flipper.atlassian.net/browse/FW-407")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("test_name", [
        "18 symbols length!",
        "Test Device Name",
        "Another Name 123",
        "T",
        "8u7Y 8a&",
        "Bu$Y 8aR",
        "wa^%$#@!()_+{}|>?",
        "Name \\ bh d / h",
        "Quotes '  ` ~",
        "Nah - ure_ad-en",
        "N!",
        "Name!",
        "Name@Home",
        "Name#1",
        "Name$Dollar",
        "Name%Percent",
        "Name^Caret",
        "Name&And",
        "Name*Star",
        "Name(Paren",
        "Name)Paren",
        "Name- Dash",
        "Name=Equal",
        "Name+Plus",
        "Name{Brace",
        "Name}Brace",
        "Name[Bracket",
        "Name]Bracket",
        "Name|Pipe",
        "Name;Semicolon",
        "Name:Colon",
        "Name\"Quote",
        "Name<Less",
        "Name>Greater",
        "Name,Comma",
        "Name.Period",
        "Name?Question",
        "Name/Slash",
        "Name\\Backslash",
    ])
    def test_api_name_post(self, settings_api: SettingsAPI, test_name):
        """Test POST /api/name endpoint"""
        with allure.step("Get current device name"):
            original = settings_api.get_name()
            allure.attach(json.dumps({"original_name": original.name}, indent=2), name="Original Name", attachment_type=allure.attachment_type.JSON)

        response = settings_api.set_name(test_name)
        assert response.result

        with allure.step("Verify name was updated"):
            verify = settings_api.get_name()
            assert verify.name == test_name

        # Restore original name
        if original.name:
            with allure.step(f"Restore original name: {original.name}"):
                settings_api.set_name(original.name)

    @allure.id("3451")
    @allure.title("Name. POST /api/name (negative)")
    @allure.issue("https://flipper.atlassian.net/browse/FW-407")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("test_name", [
        " ",
        "  ",
        "*&(^!$%(*!@%($&*^!@)($ !)(*@^$)(*!^@$ ^&!@($&*^!@(*& ^!@(^$)@(*&$!&",
        "emoji 🚀",
        "Бизи Бар",
    ])
    def test_api_name_post_invalid(self, settings_api: SettingsAPI, test_name):
        """Test POST /api/name endpoint with invalid data"""
        with allure.step("Get current device name"):
            original = settings_api.get_name()
            allure.attach(json.dumps({"original_name": original.name}, indent=2), name="Original Name", attachment_type=allure.attachment_type.JSON)

        response = settings_api.set_name_raw(test_name)
        assert response.status_code == 400

        with allure.step("Verify name was not updated"):
            verify = settings_api.get_name()
            assert verify.name == original.name


@allure.feature("5. Web Frontend")
@allure.story("Settings")
class TestSettingsAPI:
    """Test cases for Settings API endpoints"""

    @allure.id("2642")
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

    @allure.id("2643")
    @allure.title("Settings. POST /api/access")
    @allure.issue("FW-406")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("key", [
        "1234",          # valid
        "12345",         # valid
        "0000",          # valid
        "asfd",          # valid
        "asfde",         # valid
        "blablabla",     # valid
        "9999999999",    # valid
        "1234567890",    # valid
        "1a45",          # invalid - mixed
        "abcd",          # invalid - non-numeric
        "123",           # invalid - too short
        "12345678901",   # invalid - too long
        "12a34",         # invalid - mixed
        "",              # invalid - empty
        "     ",         # invalid - spaces only
    ])
    def test_api_access_post_fuzzed(self, settings_api: SettingsAPI, key):
        """Test POST /api/access endpoint with fuzzed key values"""
        response = settings_api.set_access("key", key)
        assert response.result

        verify = settings_api.get_access()
        assert verify.mode == "key"

        is_valid_key = 4 <= len(key) <= 10 and key.isdigit()
        if is_valid_key:
            with allure.step("Verify success response for valid key"):
                assert verify.key_valid is True
        else:
            with allure.step("Verify failure response for invalid key"):
                assert verify.key_valid is False

    @allure.id("2644")
    @allure.title("Settings. GET /api/display/brightness")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_display_brightness_get(self, settings_api: SettingsAPI):
        """Test GET /api/display/brightness endpoint"""
        response = settings_api.get_brightness()

        # Structure validated by pydantic
        assert response.front is not None
        assert response.back is not None

    @allure.id("2645")
    @allure.title("Settings. POST /api/display/brightness")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_display_brightness_post(self, settings_api: SettingsAPI):
        """Test POST /api/display/brightness endpoint"""
        response = settings_api.set_brightness(front="auto", back="50")

        assert response.result

    @allure.id("2646")
    @allure.title("Settings. GET /api/audio/volume")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_audio_volume_get(self, settings_api: SettingsAPI):
        """Test GET /api/audio/volume endpoint"""
        response = settings_api.get_volume()

        # Type validated by pydantic
        assert isinstance(response.volume, (int, float))

    @allure.id("2647")
    @allure.title("Settings. POST /api/audio/volume")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_audio_volume_post(self, settings_api: SettingsAPI):
        """Test POST /api/audio/volume endpoint"""
        response = settings_api.set_volume(50)

        assert response.result
