import json

import allure
import pytest


@allure.feature("5. Web Frontend")
@allure.story("Settings")
class TestNameAPI:
    """Test cases for Name API endpoints"""

    @allure.id("2712")
    @allure.title("Name. GET /api/name")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_name_get(self, api_session, web_base_url):
        """Test GET /api/name endpoint"""

        with allure.step("Make GET request to /api/name"):
            response = api_session.get(f"{web_base_url}/api/name", timeout=10)

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            name_data = response.json()
            allure.attach(
                json.dumps(name_data, indent=2),
                "Name Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields based on OpenAPI schema
            assert "name" in name_data, "Response should contain 'name' field"
            assert isinstance(name_data["name"], str), "Name should be a string"

    @allure.id("2713")
    @allure.title("Name. POST /api/name")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("test_name", ["Test Device Name", "Another Name 123", "设备名称测试", "emoji 🚀"])
    def test_api_name_post(self, api_session, web_base_url, test_name):
        """Test POST /api/name endpoint"""

        with allure.step("Get current device name"):
            get_response = api_session.get(f"{web_base_url}/api/name", timeout=10)
            original_name = None
            if get_response.status_code == 200:
                original_name = get_response.json().get("name")
                allure.attach(
                    f"Original name: {original_name}",
                    "Original Name",
                    allure.attachment_type.TEXT,
                )

        with allure.step(f"Set device name to: {test_name}"):
            response = api_session.post(
                f"{web_base_url}/api/name",
                json={"name": test_name},
                timeout=10,
            )

        with allure.step("Verify response status"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Name Set Response",
                allure.attachment_type.JSON,
            )

            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

            # Verify name was set
            with allure.step("Verify name was updated"):
                verify_response = api_session.get(
                    f"{web_base_url}/api/name", timeout=10
                )
                assert (
                    verify_response.status_code == 200
                ), f"Expected 200, got {verify_response.status_code}"
                new_name = verify_response.json().get("name")
                assert (
                    new_name == test_name
                ), f"Name should be '{test_name}', got '{new_name}'"
                allure.attach(json.dumps(verify_response.json()),
                              "Verified Name Response",
                              allure.attachment_type.JSON)

            # Restore original name if we had one
            if original_name:
                with allure.step(f"Restore original name: {original_name}"):
                    api_session.post(
                        f"{web_base_url}/api/name",
                        json={"name": original_name},
                        timeout=10,
                    )

    @allure.id("2714")
    @allure.title("Name. POST /api/name (invalid)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("test_name", ["",
                                           " ",
                                           "*&(^!$%(*!@%($&*^!@)($ !)(*@^$)(*!^@$ ^&!@($&*^!@(*& ^!@(^$)@(*&$!&",
                                           "设备名称测试",
                                           "emoji 🚀"])
    def test_api_name_post_invalid(self, api_session, web_base_url, test_name):
        """Test POST /api/name endpoint with invalid data"""

        with allure.step("Set empty device name"):
            response = api_session.post(
                f"{web_base_url}/api/name",
                json={"name": test_name},
                timeout=10,
            )

        with allure.step("Verify error response"):
            assert response.status_code in [
                400,
            ], f"Expected 400, got {response.status_code} {response.text}"


@allure.feature("5. Web Frontend")
@allure.story("Settings")
class TestSettingsAPI:
    """Test cases for Settings API endpoints"""

    @allure.id("2642")
    @allure.title("Settings. GET /api/access")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_access_get(self, api_session, web_base_url):
        """Test GET /api/access endpoint"""

        with allure.step("Make GET request to /api/access"):
            response = api_session.get(f"{web_base_url}/api/access", timeout=10)

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            access_data = response.json()
            allure.attach(
                json.dumps(access_data, indent=2),
                "Access Info Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields based on OpenAPI schema
            assert "mode" in access_data, "Response should contain 'mode' field"
            assert (
                "key_valid" in access_data
            ), "Response should contain 'key_valid' field"

            # Validate mode enum
            valid_modes = ["disabled", "enabled", "key"]
            assert (
                access_data["mode"] in valid_modes
            ), f"Mode should be one of {valid_modes}"
            assert isinstance(
                access_data["key_valid"], bool
            ), "key_valid should be a boolean"

    @allure.id("2643")
    @allure.title("Settings. POST /api/access")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_access_post(self, api_session, web_base_url):
        """Test POST /api/access endpoint"""

        with allure.step("Make POST request to /api/access with valid parameters"):
            params = {"mode": "key", "key": "12345678"}
            response = api_session.post(
                f"{web_base_url}/api/access", params=params, timeout=10
            )

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Access Set Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

    @allure.id("2644")
    @allure.title("Settings. GET /api/display/brightness")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_display_brightness_get(self, api_session, web_base_url):
        """Test GET /api/display/brightness endpoint"""

        with allure.step("Make GET request to /api/display/brightness"):
            response = api_session.get(
                f"{web_base_url}/api/display/brightness", timeout=10
            )

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            brightness_data = response.json()
            allure.attach(
                json.dumps(brightness_data, indent=2),
                "Brightness Response",
                allure.attachment_type.JSON,
            )

            # Validate fields based on OpenAPI schema
            assert "front" in brightness_data, "Response should contain 'front' field"
            assert "back" in brightness_data, "Response should contain 'back' field"

    @allure.id("2645")
    @allure.title("Settings. POST /api/display/brightness")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_display_brightness_post(self, api_session, web_base_url):
        """Test POST /api/display/brightness endpoint"""

        with allure.step("Make POST request to /api/display/brightness"):
            params = {"front": "auto", "back": "50"}
            response = api_session.post(
                f"{web_base_url}/api/display/brightness", params=params, timeout=10
            )

        with allure.step("Verify response status"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Brightness Set Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

    @allure.id("2646")
    @allure.title("Settings. GET /api/audio/volume")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_audio_volume_get(self, api_session, web_base_url):
        """Test GET /api/audio/volume endpoint"""

        with allure.step("Make GET request to /api/audio/volume"):
            response = api_session.get(f"{web_base_url}/api/audio/volume", timeout=10)

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            volume_data = response.json()
            allure.attach(
                json.dumps(volume_data, indent=2),
                "Volume Response",
                allure.attachment_type.JSON,
            )

            # Validate fields based on OpenAPI schema
            assert "volume" in volume_data, "Response should contain 'volume' field"
            assert isinstance(
                volume_data["volume"], (int, float)
            ), "Volume should be numeric"

    @allure.id("2647")
    @allure.title("Settings. POST /api/audio/volume")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_audio_volume_post(self, api_session, web_base_url):
        """Test POST /api/audio/volume endpoint"""

        with allure.step("Make POST request to /api/audio/volume"):
            params = {"volume": 50}
            response = api_session.post(
                f"{web_base_url}/api/audio/volume", params=params, timeout=10
            )

        with allure.step("Verify response status"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Volume Set Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"
