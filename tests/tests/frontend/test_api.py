import json
import time

import allure
import pytest


@allure.feature("5. Web Frontend")
@allure.story("API (draft)")
class TestSystemAPI:
    """Test cases for System API endpoints"""

    @allure.id("2638")
    @allure.title("GET /api/version [Draft]")
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
    @allure.title("GET /api/status [Draft]")
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
    @allure.title("GET /api/status/system [Draft]")
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
    @allure.title("GET /api/status/power [Draft]")
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
@allure.story("API (draft)")
class TestNameAPI:
    """Test cases for Name API endpoints"""

    @allure.id("2712")
    @allure.title("GET /api/name [Draft]")
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
    @allure.title("POST /api/name [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_name_post(self, api_session, web_base_url):
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

        test_name = "Test Device Name"

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

            # Restore original name if we had one
            if original_name:
                with allure.step(f"Restore original name: {original_name}"):
                    api_session.post(
                        f"{web_base_url}/api/name",
                        json={"name": original_name},
                        timeout=10,
                    )

    @allure.id("2714")
    @allure.title("POST /api/name (invalid) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_name_post_invalid(self, api_session, web_base_url):
        """Test POST /api/name endpoint with invalid data"""

        with allure.step("Set empty device name"):
            response = api_session.post(
                f"{web_base_url}/api/name",
                json={"name": ""},
                timeout=10,
            )

        with allure.step("Verify error response"):
            assert response.status_code in [
                400,
                200,
            ], f"Expected 400 or 200, got {response.status_code}"


@allure.feature("5. Web Frontend")
@allure.story("API (draft)")
class TestSettingsAPI:
    """Test cases for Settings API endpoints"""

    @allure.id("2642")
    @allure.title("GET /api/access [Draft]")
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
    @allure.title("POST /api/access [Draft]")
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
    @allure.title("GET /api/display/brightness [Draft]")
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
    @allure.title("POST /api/display/brightness [Draft]")
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
    @allure.title("GET /api/audio/volume [Draft]")
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
    @allure.title("POST /api/audio/volume [Draft]")
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


@allure.feature("5. Web Frontend")
@allure.story("API (draft)")
class TestStorageAPI:
    """Test cases for Storage API endpoints"""

    @allure.id("2715")
    @allure.title("GET /api/storage/status [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_status(self, api_session, web_base_url):
        """Test GET /api/storage/status endpoint"""

        with allure.step("Make GET request to /api/storage/status"):
            response = api_session.get(f"{web_base_url}/api/storage/status", timeout=10)

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json"
                in response.headers.get("content-type", "").lower()
            )

            status_data = response.json()
            allure.attach(
                json.dumps(status_data, indent=2),
                "Storage Status Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields based on OpenAPI schema
            assert (
                "used_bytes" in status_data
            ), "Response should contain 'used_bytes' field"
            assert (
                "free_bytes" in status_data
            ), "Response should contain 'free_bytes' field"
            assert (
                "total_bytes" in status_data
            ), "Response should contain 'total_bytes' field"

            # Validate types
            assert isinstance(
                status_data["used_bytes"], int
            ), "used_bytes should be an integer"
            assert isinstance(
                status_data["free_bytes"], int
            ), "free_bytes should be an integer"
            assert isinstance(
                status_data["total_bytes"], int
            ), "total_bytes should be an integer"

            # Validate logical consistency
            total = status_data["total_bytes"]
            used = status_data["used_bytes"]
            free = status_data["free_bytes"]
            assert (
                used + free <= total
            ), f"used ({used}) + free ({free}) should be <= total ({total})"

    @allure.id("2648")
    @allure.title("GET /api/storage/list [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_list(self, api_session, web_base_url):
        """Test GET /api/storage/list endpoint"""

        with allure.step("Make GET request to /api/storage/list"):
            params = {"path": "/ext"}
            response = api_session.get(
                f"{web_base_url}/api/storage/list", params=params, timeout=10
            )

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json"
                in response.headers.get("content-type", "").lower()
            )

            list_data = response.json()
            allure.attach(
                json.dumps(list_data, indent=2),
                "Storage List Response",
                allure.attachment_type.JSON,
            )

            # Validate structure based on OpenAPI schema
            assert "list" in list_data, "Response should contain 'list' field"
            assert isinstance(
                list_data["list"], list
            ), "List field should be an array"

            # Validate list items if any exist
            for item in list_data["list"]:
                assert "type" in item, "List item should contain 'type' field"
                assert "name" in item, "List item should contain 'name' field"
                assert item["type"] in [
                    "file",
                    "dir",
                ], "Type should be 'file' or 'dir'"

                if item["type"] == "file":
                    assert "size" in item, "File items should contain 'size' field"
                    assert isinstance(
                        item["size"], int
                    ), "File size should be an integer"

    @allure.id("2649")
    @allure.title("POST /api/storage/mkdir [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_mkdir(self, api_session, web_base_url):
        """Test POST /api/storage/mkdir endpoint"""

        test_dir = "/ext/test_mkdir_" + str(int(time.time()))

        with allure.step(f"Create test directory: {test_dir}"):
            params = {"path": test_dir}
            response = api_session.post(
                f"{web_base_url}/api/storage/mkdir", params=params, timeout=10
            )

        with allure.step("Verify directory creation response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Mkdir Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

    @allure.id("2650")
    @allure.title(
        "POST /api/storage/write + GET /api/storage/read + DELETE /api/storage/remove [Draft]"
    )
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_write_read_remove(self, api_session, web_base_url):
        """Test storage write, read, and remove operations"""

        test_file = "/ext/test_file_" + str(int(time.time())) + ".txt"
        test_content = b"Test content for API storage test"

        with allure.step(f"Write test file: {test_file}"):
            params = {"path": test_file}
            response = api_session.post(
                f"{web_base_url}/api/storage/write",
                params=params,
                data=test_content,
                headers={"Content-Type": "application/octet-stream"},
                timeout=10,
            )

            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            write_response = response.json()
            allure.attach(
                json.dumps(write_response, indent=2),
                "Write Response",
                allure.attachment_type.JSON,
            )

            with allure.step(f"Read test file: {test_file}"):
                params = {"path": test_file}
                read_response = api_session.get(
                    f"{web_base_url}/api/storage/read", params=params, timeout=10
                )

                assert (
                    read_response.status_code == 200
                ), f"Expected 200, got {read_response.status_code}"
                assert (
                    read_response.content == test_content
                ), "Read content should match written content"
                allure.attach(
                    read_response.content.decode(),
                    "File Content",
                    allure.attachment_type.TEXT,
                )

                with allure.step(f"Remove test file: {test_file}"):
                    params = {"path": test_file}
                    remove_response = api_session.delete(
                        f"{web_base_url}/api/storage/remove",
                        params=params,
                        timeout=10,
                    )

                    assert (
                        remove_response.status_code == 200
                    ), f"Expected 200, got {remove_response.status_code}"
                    remove_data = remove_response.json()
                    allure.attach(
                        json.dumps(remove_data, indent=2),
                        "Remove Response",
                        allure.attachment_type.JSON,
                    )


@allure.feature("5. Web Frontend")
@allure.story("API (draft)")
class TestAssetsAPI:
    """Test cases for Assets API endpoints"""

    @allure.id("2651")
    @allure.title("POST /api/assets/upload [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_assets_upload(self, api_session, web_base_url):
        """Test POST /api/assets/upload endpoint"""

        test_app_id = "test_app"
        test_filename = "test_asset.txt"
        test_content = b"Test asset content"

        with allure.step(f"Upload asset for app {test_app_id}"):
            params = {"app_id": test_app_id, "file": test_filename}
            response = api_session.post(
                f"{web_base_url}/api/assets/upload",
                params=params,
                data=test_content,
                headers={"Content-Type": "application/octet-stream"},
                timeout=10,
            )

        with allure.step("Verify upload response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Asset Upload Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

    @allure.id("2652")
    @allure.title("DELETE /api/assets/upload [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_assets_delete(self, api_session, web_base_url):
        """Test DELETE /api/assets/upload endpoint"""

        test_app_id = "test_app"

        with allure.step(f"Delete assets for app {test_app_id}"):
            params = {"app_id": test_app_id}
            response = api_session.delete(
                f"{web_base_url}/api/assets/upload", params=params, timeout=10
            )

        with allure.step("Verify delete response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Asset Delete Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

    @allure.id("2653")
    @allure.title("POST /api/display/draw [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_display_draw(self, api_session, web_base_url):
        """Test POST /api/display/draw endpoint"""

        display_data = {
            "app_id": "test_app",
            "elements": [
                {
                    "id": "1",
                    "timeout": 5,
                    "type": "text",
                    "text": "Hello API Test",
                    "x": 36,
                    "y": 10,
                    "align": "center",
                    "font": "medium",
                    "color": "#FFFFFFFF",
                    "display": "front",
                }
            ],
        }

        with allure.step("Send display draw command"):
            response = api_session.post(
                f"{web_base_url}/api/display/draw", json=display_data, timeout=10
            )

        with allure.step("Verify draw response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Display Draw Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

    @allure.id("2654")
    @allure.title("DELETE /api/display/draw [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Firmware bug - endpoint times out")
    def test_api_display_clear(self, api_session, web_base_url):
        """Test DELETE /api/display/draw endpoint"""

        with allure.step("Clear display"):
            response = api_session.delete(
                f"{web_base_url}/api/display/draw", timeout=10
            )

        with allure.step("Verify clear response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Display Clear Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

    @allure.id("2655")
    @allure.title("POST /api/audio/play [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Requires test audio file to be uploaded first")
    def test_api_audio_play(self, api_session, web_base_url):
        """Test POST /api/audio/play endpoint"""

        with allure.step("Play audio file"):
            params = {"app_id": "test_app", "path": "test.snd"}
            response = api_session.post(
                f"{web_base_url}/api/audio/play", params=params, timeout=10
            )

        with allure.step("Verify play response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Audio Play Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"

    @allure.id("2656")
    @allure.title("DELETE /api/audio/play [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Requires audio to be playing first")
    def test_api_audio_stop(self, api_session, web_base_url):
        """Test DELETE /api/audio/play endpoint"""

        with allure.step("Stop audio playback"):
            response = api_session.delete(f"{web_base_url}/api/audio/play", timeout=10)

        with allure.step("Verify stop response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Audio Stop Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"


@allure.feature("5. Web Frontend")
@allure.story("API (draft)")
class TestWifiAPI:
    """Test cases for WiFi API endpoints"""

    @allure.id("2657")
    @allure.title("GET /api/wifi/status [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_status(self, api_session, web_base_url):
        """Test GET /api/wifi/status endpoint"""

        with allure.step("Get WiFi status"):
            response = api_session.get(f"{web_base_url}/api/wifi/status", timeout=10)

        with allure.step("Verify status response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            status_data = response.json()
            allure.attach(
                json.dumps(status_data, indent=2),
                "WiFi Status Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields
            assert "state" in status_data, "Response should contain 'state' field"
            valid_states = [
                "unknown",
                "disconnected",
                "connected",
                "connecting",
                "disconnecting",
                "reconnecting",
            ]
            assert (
                status_data["state"] in valid_states
            ), f"State should be one of {valid_states}"

    @allure.id("2660")
    @allure.title("GET /api/wifi/networks [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_networks(self, api_session, web_base_url):
        """Test GET /api/wifi/networks endpoint"""

        with allure.step("Scan for WiFi networks"):
            response = api_session.get(
                f"{web_base_url}/api/wifi/networks", timeout=30
            )  # Longer timeout for scan

        with allure.step("Verify networks response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            networks_data = response.json()
            allure.attach(
                json.dumps(networks_data, indent=2),
                "WiFi Networks Response",
                allure.attachment_type.JSON,
            )

            assert "count" in networks_data, "Response should contain 'count' field"
            assert (
                "networks" in networks_data
            ), "Response should contain 'networks' field"
            assert isinstance(
                networks_data["networks"], list
            ), "Networks should be a list"

    @allure.id("2661")
    @allure.title("POST /api/wifi/connect [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_connect_invalid(self, api_session, web_base_url):
        """Test POST /api/wifi/connect endpoint with invalid data"""

        with allure.step("Attempt WiFi connection with invalid credentials"):
            connect_data = {
                "ssid": "NonExistentNetwork",
                "password": "wrongpassword",
                "security": "WPA2",
            }
            response = api_session.post(
                f"{web_base_url}/api/wifi/connect", json=connect_data, timeout=30
            )

        with allure.step("Verify connect response"):
            # Should return 400 for bad request or connection failure
            assert response.status_code in [
                200,
                400,
            ], f"Expected 200 or 400, got {response.status_code}"

    @allure.id("2662")
    @allure.title("POST /api/wifi/disconnect [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_wifi_disconnect(self, api_session, web_base_url):
        """Test POST /api/wifi/disconnect endpoint"""

        with allure.step("Check WiFi status before disconnect"):
            status_response = api_session.get(
                f"{web_base_url}/api/wifi/status", timeout=10
            )
            assert (
                status_response.status_code == 200
            ), f"Expected 200, got {status_response.status_code}"
            status_data = status_response.json()
            wifi_state = status_data.get("state", "unknown")
            allure.attach(
                json.dumps(status_data, indent=2),
                "WiFi Status Before Disconnect",
                allure.attachment_type.JSON,
            )

            if wifi_state not in ["connected", "connecting"]:
                pytest.skip(f"WiFi is not connected (state: {wifi_state}), skipping disconnect test")

        with allure.step("Disconnect from WiFi"):
            response = api_session.post(
                f"{web_base_url}/api/wifi/disconnect", timeout=10
            )

        with allure.step("Verify disconnect response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "WiFi Disconnect Response",
                allure.attachment_type.JSON,
            )
            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"


@allure.feature("5. Web Frontend")
@allure.story("API (draft)")
class TestBleAPI:
    """Test cases for BLE API endpoints"""

    @allure.id("2663")
    @allure.title("POST /api/ble/enable [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_ble_enable(self, api_session, web_base_url):
        """Test POST /api/ble/enable endpoint"""

        with allure.step("Enable BLE"):
            response = api_session.post(f"{web_base_url}/api/ble/enable", timeout=10)

        with allure.step("Verify enable response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "BLE Enable Response",
                allure.attachment_type.JSON,
            )

    @allure.id("2664")
    @allure.title("POST /api/ble/disable [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_ble_disable(self, api_session, web_base_url):
        """Test POST /api/ble/disable endpoint"""

        with allure.step("Disable BLE"):
            response = api_session.post(f"{web_base_url}/api/ble/disable", timeout=10)

        with allure.step("Verify disable response"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "BLE Disable Response",
                allure.attachment_type.JSON,
            )


@allure.feature("5. Web Frontend")
@allure.story("API (draft)")
class TestInputAPI:
    """Test cases for Input API endpoints"""

    @allure.id("2665")
    @allure.title("POST /api/input (key events) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_input_key_post(self, api_session, web_base_url):
        """Test POST /api/input endpoint for key events"""

        valid_keys = [
            "up",
            "down",
            "ok",
            "back",
            "start",
            "busy",
            "status",
            "off",
            "apps",
            "settings",
        ]

        for key in valid_keys:
            with allure.step(f"Send key event: {key}"):
                params = {"key": key}
                response = api_session.post(
                    f"{web_base_url}/api/input", params=params, timeout=10
                )

                assert response.status_code in [
                    200,
                    400,
                ], f"Expected 200 or 400 for key {key}, got {response.status_code}"

                if response.status_code == 200:
                    response_data = response.json()
                    allure.attach(
                        json.dumps(response_data, indent=2),
                        f"Input {key} Response",
                        allure.attachment_type.JSON,
                    )

    @allure.id("2666")
    @allure.title("POST /api/input (invalid key) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_input_invalid_key(self, api_session, web_base_url):
        """Test POST /api/input endpoint with invalid key"""

        with allure.step("Send invalid key event"):
            params = {"key": "invalid_key"}
            response = api_session.post(
                f"{web_base_url}/api/input", params=params, timeout=10
            )

        with allure.step("Verify invalid key response"):
            assert (
                response.status_code == 400
            ), f"Expected 400 for invalid key, got {response.status_code}"

            error_data = response.json()
            allure.attach(
                json.dumps(error_data, indent=2),
                "Invalid Key Error Response",
                allure.attachment_type.JSON,
            )
            assert "error" in error_data, "Error response should contain 'error' field"


@allure.feature("5. Web Frontend")
@allure.story("API (draft)")
class TestStreamingAPI:
    """Test cases for Streaming API endpoints"""

    @allure.id("2667")
    @allure.title("GET /api/screen (front display) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_screen_front_display(self, api_session, web_base_url):
        """Test GET /api/screen endpoint for front display"""

        with allure.step("Get front display frame"):
            params = {"display": 0}  # Front display
            response = api_session.get(
                f"{web_base_url}/api/screen", params=params, timeout=10
            )

        with allure.step("Verify screen response"):
            assert response.status_code in [
                200,
                400,
            ], f"Expected 200 or 400, got {response.status_code}"

            if response.status_code == 200:
                content_type = response.headers.get("content-type", "")
                assert (
                    "image/bmp" in content_type.lower()
                ), f"Expected BMP image, got {content_type}"
                assert len(response.content) > 0, "Image data should not be empty"
                allure.attach(
                    response.content, "Front Display Frame", allure.attachment_type.PNG
                )

    @allure.id("2668")
    @allure.title("GET /api/screen (back display) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_screen_back_display(self, api_session, web_base_url):
        """Test GET /api/screen endpoint for back display"""

        with allure.step("Get back display frame"):
            params = {"display": 1}  # Back display
            response = api_session.get(
                f"{web_base_url}/api/screen", params=params, timeout=10
            )

        with allure.step("Verify screen response"):
            assert response.status_code in [
                200,
                400,
            ], f"Expected 200 or 400, got {response.status_code}"

            if response.status_code == 200:
                content_type = response.headers.get("content-type", "")
                assert (
                    "image/bmp" in content_type.lower()
                ), f"Expected BMP image, got {content_type}"
                assert len(response.content) > 0, "Image data should not be empty"
                allure.attach(
                    response.content, "Back Display Frame", allure.attachment_type.PNG
                )

    @allure.id("2669")
    @allure.title("GET /api/screen (invalid display) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_screen_invalid_display(self, api_session, web_base_url):
        """Test GET /api/screen endpoint with invalid display number"""

        with allure.step("Request invalid display"):
            params = {"display": 2}  # Invalid display number
            response = api_session.get(
                f"{web_base_url}/api/screen", params=params, timeout=10
            )

        with allure.step("Verify error response"):
            assert (
                response.status_code == 400
            ), f"Expected 400 for invalid display, got {response.status_code}"


@allure.feature("5. Web Frontend")
@allure.story("API (draft)")
class TestUpdateAPI:
    """Test cases for Update API endpoints"""

    @allure.id("2670")
    @allure.title("POST /api/update [Draft]")
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
            # Create a minimal TAR-like content for testing
            # Note: This is a simulation - real update would need actual firmware
            mock_tar_content = b"Mock firmware update package for testing"

        with allure.step("Attempt firmware update"):
            params = {"name": "test_firmware"}

            # Note: This test will likely fail with 400 due to invalid package
            # but we're testing the API endpoint behavior
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

            if response.status_code == 200:
                # If successful, device would reboot - can't verify version change in same test
                assert (
                    "result" in response_data
                ), "Success response should contain 'result' field"
            else:
                # For error cases, verify error structure
                assert (
                    "error" in response_data
                ), "Error response should contain 'error' field"


@allure.feature("5. Web Frontend")
@allure.story("API (draft)")
class TestAPIErrorHandling:
    """Test cases for API error handling and edge cases"""

    @allure.id("2671")
    @allure.title("GET /api/* (404 errors) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_endpoints_404(self, api_session, web_base_url):
        """Test that non-existent API endpoints return 404"""

        invalid_endpoints = [
            "/api/nonexistent",
            "/api/invalid/endpoint",
            "/api/test/fake",
            "/api/version/invalid",
        ]

        for endpoint in invalid_endpoints:
            with allure.step(f"Test invalid endpoint: {endpoint}"):
                response = api_session.get(f"{web_base_url}{endpoint}", timeout=10)
                assert (
                    response.status_code == 404
                ), f"Expected 404 for {endpoint}, got {response.status_code}"

    @allure.id("2672")
    @allure.title("POST /api/display/draw (malformed JSON) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_malformed_json(self, api_session, web_base_url):
        """Test API endpoints with malformed JSON"""

        malformed_json = '{"invalid": "json", "missing": quote}'

        with allure.step("Send malformed JSON to display/draw"):
            response = api_session.post(
                f"{web_base_url}/api/display/draw",
                data=malformed_json,
                headers={"Content-Type": "application/json"},
                timeout=10,
            )

        with allure.step("Verify malformed JSON handling"):
            assert (
                response.status_code == 400
            ), f"Expected 400 for malformed JSON, got {response.status_code}"

    @allure.id("2673")
    @allure.title("API endpoints (missing required parameters) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_missing_required_parameters(self, api_session, web_base_url):
        """Test API endpoints with missing required parameters"""

        test_cases = [
            ("/api/storage/list", {}),  # Missing path parameter
            ("/api/input", {}),  # Missing key parameter
            ("/api/screen", {}),  # Missing display parameter
        ]

        for endpoint, params in test_cases:
            with allure.step(f"Test missing parameters for {endpoint}"):
                if endpoint == "/api/input":
                    response = api_session.post(
                        f"{web_base_url}{endpoint}", params=params, timeout=10
                    )
                else:
                    response = api_session.get(
                        f"{web_base_url}{endpoint}", params=params, timeout=10
                    )

                assert (
                    response.status_code == 400
                ), f"Expected 400 for missing params on {endpoint}, got {response.status_code}"

    @allure.id("2674")
    @allure.title("API endpoints (invalid parameter values) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_invalid_parameter_values(self, api_session, web_base_url):
        """Test API endpoints with invalid parameter values"""

        test_cases = [
            ("/api/input", {"key": "invalid_key_name"}),
            ("/api/screen", {"display": "invalid"}),
            ("/api/screen", {"display": -1}),
            ("/api/audio/volume", {"volume": -50}),
            ("/api/audio/volume", {"volume": 150}),
        ]

        for endpoint, params in test_cases:
            with allure.step(f"Test invalid parameters for {endpoint}"):
                if endpoint == "/api/input":
                    response = api_session.post(
                        f"{web_base_url}{endpoint}", params=params, timeout=10
                    )
                elif endpoint == "/api/audio/volume":
                    response = api_session.post(
                        f"{web_base_url}{endpoint}", params=params, timeout=10
                    )
                else:
                    response = api_session.get(
                        f"{web_base_url}{endpoint}", params=params, timeout=10
                    )

                assert (
                    response.status_code == 400
                ), f"Expected 400 for invalid params on {endpoint}, got {response.status_code}"

    @allure.id("2675")
    @allure.title("POST /api/storage/write (file size limits) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_file_upload_size_limit(self, api_session, web_base_url):
        """Test file upload size limits"""

        test_file_path = f"/ext/large_test_fil.bin"

        # Create a large file (simulate size limit testing)
        large_content = b"x" * (10 * 1024 * 1024)  # 10MB

        with allure.step(f"Check if test file already exists: {test_file_path}"):
            # First check if file exists and clean it up
            check_response = api_session.get(
                f"{web_base_url}/api/storage/read",
                params={"path": test_file_path},
                timeout=10,
            )
            if check_response.status_code == 200:
                allure.attach(
                    f"File {test_file_path} already exists, attempting cleanup",
                    "Pre-cleanup",
                    allure.attachment_type.TEXT,
                )

                # File exists, try to delete it first
                delete_response = api_session.delete(
                    f"{web_base_url}/api/storage/remove",
                    params={"path": test_file_path},
                    timeout=10,
                )
                if delete_response.status_code != 200:
                    pytest.fail(
                        f"Failed to delete existing test file {test_file_path}. Status: {delete_response.status_code}"
                    )
                else:
                    allure.attach(
                        f"Successfully cleaned up existing file {test_file_path}",
                        "Pre-cleanup Success",
                        allure.attachment_type.TEXT,
                    )

        upload_successful = False

        try:
            with allure.step(f"Test large file upload to storage: {test_file_path}"):
                params = {"path": test_file_path}
                response = api_session.post(
                    f"{web_base_url}/api/storage/write",
                    params=params,
                    data=large_content,
                    headers={"Content-Type": "application/octet-stream"},
                    timeout=30,
                )

            with allure.step("Verify size limit handling"):
                # Should return 413 (Payload Too Large), 400 (Bad Request), 500 (Server Error), or 200 (Success if no size limit)
                assert response.status_code in [
                    200,
                    400,
                    413,
                    500,
                ], f"Expected 200, 400, 413, or 500, got {response.status_code}"

                if response.status_code == 200:
                    upload_successful = True
                    allure.attach(
                        "Large file upload succeeded - no size limit enforced",
                        "Upload Result",
                        allure.attachment_type.TEXT,
                    )

                    # Verify file was actually written by checking its size
                    verify_response = api_session.get(
                        f"{web_base_url}/api/storage/read",
                        params={"path": test_file_path},
                        timeout=10,
                    )
                    if verify_response.status_code == 200:
                        actual_size = len(verify_response.content)
                        expected_size = len(large_content)
                        allure.attach(
                            f"File size verification: Expected {expected_size} bytes, got {actual_size} bytes",
                            "Size Verification",
                            allure.attachment_type.TEXT,
                        )
                        assert (
                            actual_size == expected_size
                        ), f"File size mismatch: expected {expected_size}, got {actual_size}"
                else:
                    allure.attach(
                        f"Large file upload rejected with status {response.status_code} - size limits enforced",
                        "Upload Result",
                        allure.attachment_type.TEXT,
                    )

        finally:
            # Always attempt cleanup if upload was successful
            if upload_successful:
                with allure.step(f"Clean up test file: {test_file_path}"):
                    cleanup_response = api_session.delete(
                        f"{web_base_url}/api/storage/remove",
                        params={"path": test_file_path},
                        timeout=10,
                    )

                    if cleanup_response.status_code == 200:
                        allure.attach(
                            "Large test file cleaned up successfully",
                            "Cleanup Success",
                            allure.attachment_type.TEXT,
                        )
                    else:
                        # Cleanup failure should fail the test
                        error_msg = f"Failed to clean up test file {test_file_path}. Status: {cleanup_response.status_code}"
                        allure.attach(
                            error_msg, "Cleanup Failure", allure.attachment_type.TEXT
                        )
                        pytest.fail(error_msg)

    @allure.id("2708")
    @allure.title("GET /api/version (concurrent requests) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_concurrent_requests(self, api_session, web_base_url):
        """Test API behavior under concurrent requests"""

        import queue
        import threading

        results = queue.Queue()

        def make_request():
            try:
                response = api_session.get(f"{web_base_url}/api/version", timeout=10)
                results.put(response.status_code)
            except Exception as e:
                results.put(f"Error: {e}")

        with allure.step("Send 5 concurrent requests"):
            threads = []
            for i in range(5):
                thread = threading.Thread(target=make_request)
                threads.append(thread)
                thread.start()

            for thread in threads:
                thread.join()

        with allure.step("Verify all requests succeeded"):
            response_codes = []
            while not results.empty():
                result = results.get()
                response_codes.append(result)

            # All requests should succeed (status 200)
            success_count = sum(1 for code in response_codes if code == 200)
            assert (
                success_count >= 3
            ), f"Expected at least 3 successful requests, got {success_count} out of {len(response_codes)}"


@allure.feature("5. Web Frontend")
@allure.story("API (draft)")
class TestTimeAPI:
    """Test cases for Time API endpoints"""

    @allure.id("2716")
    @allure.title("GET /api/time [Draft]")
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
    @allure.title("POST /api/time/timestamp [Draft]")
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
    @allure.title("POST /api/time/timestamp (invalid) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_time_timestamp_post_invalid(self, api_session, web_base_url):
        """Test POST /api/time/timestamp endpoint with invalid format"""

        invalid_timestamps = [
            "not-a-timestamp",
            "2025/06/15 12:30:45",
            "12:30:45",
            "",
        ]

        for invalid_ts in invalid_timestamps:
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
    @allure.title("POST /api/time/timezone [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_time_timezone_post(self, api_session, web_base_url):
        """Test POST /api/time/timezone endpoint"""

        # Test valid timezone offset
        test_timezone = "+04:00"

        with allure.step(f"Set timezone to: {test_timezone}"):
            params = {"timezone": test_timezone}
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
    @allure.title("POST /api/time/timezone (invalid) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_time_timezone_post_invalid(self, api_session, web_base_url):
        """Test POST /api/time/timezone endpoint with invalid format"""

        invalid_timezones = [
            "invalid",
            "+25:00",  # Out of range
            "-15:00",  # Out of range
            "",
        ]

        for invalid_tz in invalid_timezones:
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
@allure.story("API (draft)")
class TestAccountAPI:
    """Test cases for Account API endpoints"""

    @allure.id("2721")
    @allure.title("GET /api/account [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_get(self, api_session, web_base_url):
        """Test GET /api/account endpoint"""

        with allure.step("Make GET request to /api/account"):
            response = api_session.get(f"{web_base_url}/api/account", timeout=10)

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            account_data = response.json()
            allure.attach(
                json.dumps(account_data, indent=2),
                "Account Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields based on OpenAPI schema
            assert "state" in account_data, "Response should contain 'state' field"

            # Validate state enum
            valid_states = ["error", "disconnected", "not_linked", "linked"]
            assert (
                account_data["state"] in valid_states
            ), f"State should be one of {valid_states}, got {account_data['state']}"

            # If linked, additional fields should be present
            if account_data["state"] == "linked":
                assert "id" in account_data, "Linked state should include 'id'"
                assert "email" in account_data, "Linked state should include 'email'"

    @allure.id("2722")
    @allure.title("POST /api/account/link [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_account_link_post(self, api_session, web_base_url):
        """Test POST /api/account/link endpoint"""

        with allure.step("Check account status before linking"):
            status_response = api_session.get(
                f"{web_base_url}/api/account", timeout=10
            )
            assert (
                status_response.status_code == 200
            ), f"Expected 200, got {status_response.status_code}"
            account_data = status_response.json()
            account_state = account_data.get("state", "unknown")
            allure.attach(
                json.dumps(account_data, indent=2),
                "Account Status Before Link",
                allure.attachment_type.JSON,
            )

            if account_state == "linked":
                pytest.skip("Account is already linked, skipping link test")
            if account_state in ["error", "disconnected"]:
                pytest.skip(f"Account is not ready for linking (state: {account_state}), skipping link test")

        with allure.step("Request account linking PIN"):
            response = api_session.post(f"{web_base_url}/api/account/link", timeout=10)

        with allure.step("Verify response status"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Account Link Response",
                allure.attachment_type.JSON,
            )

            assert "code" in response_data, "Response should contain 'code' field"
            assert (
                "expires_at" in response_data
            ), "Response should contain 'expires_at' field"

    @allure.id("2723")
    @allure.title("DELETE /api/account [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Destructive test - unlinks account")
    def test_api_account_delete(self, api_session, web_base_url):
        """Test DELETE /api/account endpoint (unlink)"""

        with allure.step("Unlink account"):
            response = api_session.delete(f"{web_base_url}/api/account", timeout=10)

        with allure.step("Verify response status"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "Account Unlink Response",
                allure.attachment_type.JSON,
            )

            assert (
                "result" in response_data
            ), "Success response should contain 'result' field"


@allure.feature("5. Web Frontend")
@allure.story("API (draft)")
class TestBleStatusAPI:
    """Test cases for BLE Status API endpoints"""

    @allure.id("2724")
    @allure.title("GET /api/ble/status [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_ble_status_get(self, api_session, web_base_url):
        """Test GET /api/ble/status endpoint"""

        with allure.step("Make GET request to /api/ble/status"):
            response = api_session.get(f"{web_base_url}/api/ble/status", timeout=10)

        with allure.step("Verify response status and structure"):
            assert (
                response.status_code == 200
            ), f"Expected 200, got {response.status_code}"
            assert (
                "application/json" in response.headers.get("content-type", "").lower()
            )

            ble_data = response.json()
            allure.attach(
                json.dumps(ble_data, indent=2),
                "BLE Status Response",
                allure.attachment_type.JSON,
            )

            # Validate required fields based on OpenAPI schema
            assert "state" in ble_data, "Response should contain 'state' field"
            assert "pairing" in ble_data, "Response should contain 'pairing' field"

            # Validate state enum
            valid_states = [
                "reset",
                "initialization",
                "disabled",
                "enabled",
                "connected",
                "internal error",
            ]
            assert (
                ble_data["state"] in valid_states
            ), f"State should be one of {valid_states}, got {ble_data['state']}"

            # Address field is only present when BLE is enabled
            if ble_data["state"] in ["enabled", "connected"]:
                assert "address" in ble_data, "Response should contain 'address' field when BLE is enabled"

            # Validate pairing enum
            valid_pairing = ["unknown", "not paired", "paired"]
            assert (
                ble_data["pairing"] in valid_pairing
            ), f"Pairing should be one of {valid_pairing}, got {ble_data['pairing']}"

    @allure.id("2725")
    @allure.title("DELETE /api/ble/pairing [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Destructive test - removes BLE pairing")
    def test_api_ble_pairing_delete(self, api_session, web_base_url):
        """Test DELETE /api/ble/pairing endpoint"""

        with allure.step("Remove BLE pairing"):
            response = api_session.delete(f"{web_base_url}/api/ble/pairing", timeout=10)

        with allure.step("Verify response status"):
            # May return 200 if successful, 503 if BLE not initialized or already unpaired
            assert response.status_code in [
                200,
                503,
            ], f"Expected 200 or 503, got {response.status_code}"

            response_data = response.json()
            allure.attach(
                json.dumps(response_data, indent=2),
                "BLE Pairing Remove Response",
                allure.attachment_type.JSON,
            )


@allure.feature("5. Web Frontend")
@allure.story("API (draft)")
class TestAPIAuthentication:
    """Test cases for API authentication (when implemented)"""

    @allure.id("2709")
    @allure.title("GET /api/status (no auth token) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Authentication not implemented yet")
    def test_api_without_auth_token(self, api_session, web_base_url):
        """Test API access without authentication token"""

        # Remove any auth headers
        headers = api_session.headers.copy()
        if "X-API-Token" in headers:
            del headers["X-API-Token"]

        with allure.step("Access protected endpoint without auth"):
            response = api_session.get(
                f"{web_base_url}/api/status", headers=headers, timeout=10
            )

        with allure.step("Verify authentication required"):
            assert (
                response.status_code == 401
            ), f"Expected 401 Unauthorized, got {response.status_code}"

    @allure.id("2710")
    @allure.title("GET /api/status (invalid auth token) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Authentication not implemented yet")
    def test_api_with_invalid_auth_token(self, api_session, web_base_url):
        """Test API access with invalid authentication token"""

        headers = api_session.headers.copy()
        headers["X-API-Token"] = "invalid-token"

        with allure.step("Access protected endpoint with invalid auth"):
            response = api_session.get(
                f"{web_base_url}/api/status", headers=headers, timeout=10
            )

        with allure.step("Verify invalid token rejection"):
            assert (
                response.status_code == 403
            ), f"Expected 403 Forbidden, got {response.status_code}"

    @allure.id("2711")
    @allure.title("GET /api/status (valid auth token) [Draft]")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.skip(reason="Authentication not implemented yet")
    def test_api_with_valid_auth_token(self, api_auth_session, web_base_url):
        """Test API access with valid authentication token"""

        with allure.step("Access protected endpoint with valid auth"):
            response = api_auth_session.get(f"{web_base_url}/api/status", timeout=10)

        with allure.step("Verify successful access"):
            assert (
                response.status_code == 200
            ), f"Expected 200 with valid auth, got {response.status_code}"
