import allure
import pytest

from utils import (
    api_get,
    api_post,
    attach_json,
    assert_has_fields,
    assert_field_in,
    assert_field_type,
)


@allure.feature("5. Web Frontend")
@allure.story("Settings")
class TestNameAPI:
    """Test cases for Name API endpoints"""

    @allure.id("2883")
    @allure.title("Name. GET /api/name")
    @allure.issue("https://flipper.atlassian.net/browse/FW-407")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_name_get(self, api_session, web_base_url):
        """Test GET /api/name endpoint"""

        with allure.step("Make GET request to /api/name"):
            response = api_get(api_session, web_base_url, "/api/name")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("name")
            response.assert_field_type("name", str).attach_to_allure("Name Response")

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
        "Name \ bh d / h",
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
    def test_api_name_post(self, api_session, web_base_url, test_name):
        """Test POST /api/name endpoint"""

        with allure.step("Get current device name"):
            get_response = api_get(api_session, web_base_url, "/api/name")
            original_name = None
            if get_response.status_code == 200:
                original_name = get_response.get_field("name")
                attach_json({"original_name": original_name}, "Original Name")

        with allure.step(f"Set device name to: {test_name}"):
            response = api_post(
                api_session, web_base_url, "/api/name",
                json={"name": test_name}
            )

        with allure.step("Verify response status"):
            response.assert_ok()
            response.assert_has_fields("result").attach_to_allure("Name Set Response")

            # Verify name was set
            with allure.step("Verify name was updated"):
                verify_response = api_get(api_session, web_base_url, "/api/name")
                verify_response.assert_ok()
                new_name = verify_response.get_field("name")
                assert (
                    new_name == test_name
                ), f"Name should be '{test_name}', got '{new_name}'"
                verify_response.attach_to_allure("Verified Name Response")

            # Restore original name if we had one
            if original_name:
                with allure.step(f"Restore original name: {original_name}"):
                    api_post(
                        api_session, web_base_url, "/api/name",
                        json={"name": original_name}
                    )

    @allure.id("3451")
    @allure.title("Name. POST /api/name (negative)")
    @allure.issue("https://flipper.atlassian.net/browse/FW-407")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("test_name", [
        " ",
        "  ",
        "*&(^!$%(*!@%($&*^!@)($ !)(*@^$)(*!^@$ ^&!@($&*^!@(*& ^!@(^$)@(*&$!&",
        "设备名称测试",
        "emoji 🚀",
        "Бизи Бар",
    ])
    def test_api_name_post_invalid(self, api_session, web_base_url, test_name):
        """Test POST /api/name endpoint with invalid data"""

        with allure.step("Get current device name"):
            get_response = api_get(api_session, web_base_url, "/api/name")
            original_name = None
            if get_response.status_code == 200:
                original_name = get_response.get_field("name")
                attach_json({"original_name": original_name}, "Original Name")

        with allure.step("Set invalid device name"):
            response = api_post(
                api_session, web_base_url, "/api/name",
                json={"name": test_name}
            )

        with allure.step("Verify error response"):
            response.assert_bad_request()

        with allure.step("Verify name was not updated"):
            verify_response = api_get(api_session, web_base_url, "/api/name")
            verify_response.assert_ok()
            new_name = verify_response.get_field("name")
            assert (
                new_name == original_name
            ), f"Name should be '{original_name}', got '{new_name}'"
            verify_response.attach_to_allure("Verified Name Response")

@allure.feature("5. Web Frontend")
@allure.story("Settings")
class TestSettingsAPI:
    """Test cases for Settings API endpoints"""

    @allure.id("2642")
    @allure.title("Settings. GET /api/access")
    @allure.issue("FW-406")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_access_get(self, api_session, web_base_url):
        """Test GET /api/access endpoint"""

        with allure.step("Make GET request to /api/access"):
            response = api_get(api_session, web_base_url, "/api/access")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("mode", "key_valid").attach_to_allure("Access Info Response")

            data = response.json()
            assert_field_in(data, "mode", ["disabled", "enabled", "key"])
            assert_field_type(data, "key_valid", bool)

    @allure.id("2643")
    @allure.title("Settings. POST /api/access")
    @allure.issue("FW-406")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("key", [
        "1234",          # valid
        "12345",  # valid
        "0000",  # valid
        "asfd",       # valid
        "asfde",  # valid
        "blablabla",  # valid
        "9999999999",  # valid
        "1234567890",  # valid
        "1a45",       # invalid - mixed
        "🔑🔒",       # invalid - emoji
        "ключ",        # invalid - non-latin
        "１２３４５",    # invalid - full-width
        "汉字汉字汉字",        # invalid - Chinese characters
        "abcd",         # invalid - non-numeric
        "123",          # invalid - too short
        "12345678901",  # invalid - too long
        "12a34",       # invalid - mixed
        "",             # invalid - empty
        "     ",       # invalid - spaces only
    ])
    def test_api_access_post_fuzzed(self, api_session, web_base_url, key):
        """Test POST /api/access endpoint with fuzzed key values"""

        with allure.step(f"Make POST request to /api/access with key: {key!r}"):
            response = api_post(
                api_session, web_base_url, "/api/access",
                params={"mode": "key", "key": key}
            )

        response.assert_ok().assert_json_content_type()
        response.assert_has_fields("result").attach_to_allure("Access Set Response")
        verify_response = api_get(api_session, web_base_url, "/api/access")
        verify_response.assert_ok()
        verify_data = verify_response.json()
        assert verify_data["mode"] == "key", f"Expected mode 'key', got '{verify_data['mode']}'"

        is_valid_key = (
                4 <= len(key) <= 10
                and key.isdigit()
        )
        if is_valid_key:
            with allure.step("Verify success response for valid key"):
                assert verify_data["key_valid"] is True, "Expected key_valid to be True"
        else:
            with allure.step("Verify failure response for invalid key"):
                assert verify_data["key_valid"] is False, "Expected key_valid to be False"


    @allure.id("2644")
    @allure.title("Settings. GET /api/display/brightness")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_display_brightness_get(self, api_session, web_base_url):
        """Test GET /api/display/brightness endpoint"""

        with allure.step("Make GET request to /api/display/brightness"):
            response = api_get(api_session, web_base_url, "/api/display/brightness")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("front", "back").attach_to_allure("Brightness Response")

    @allure.id("2645")
    @allure.title("Settings. POST /api/display/brightness")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_display_brightness_post(self, api_session, web_base_url):
        """Test POST /api/display/brightness endpoint"""

        with allure.step("Make POST request to /api/display/brightness"):
            response = api_post(
                api_session, web_base_url, "/api/display/brightness",
                params={"front": "auto", "back": "50"}
            )

        with allure.step("Verify response status"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("result").attach_to_allure("Brightness Set Response")

    @allure.id("2646")
    @allure.title("Settings. GET /api/audio/volume")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_audio_volume_get(self, api_session, web_base_url):
        """Test GET /api/audio/volume endpoint"""

        with allure.step("Make GET request to /api/audio/volume"):
            response = api_get(api_session, web_base_url, "/api/audio/volume")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("volume").attach_to_allure("Volume Response")

            data = response.json()
            assert_field_type(data, "volume", (int, float))

    @allure.id("2647")
    @allure.title("Settings. POST /api/audio/volume")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_audio_volume_post(self, api_session, web_base_url):
        """Test POST /api/audio/volume endpoint"""

        with allure.step("Make POST request to /api/audio/volume"):
            response = api_post(
                api_session, web_base_url, "/api/audio/volume",
                params={"volume": 50}
            )

        with allure.step("Verify response status"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("result").attach_to_allure("Volume Set Response")
