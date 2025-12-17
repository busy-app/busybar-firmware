import json
import time

import allure
import pytest


@allure.feature("5. Web Frontend")
@allure.story("Storage")
class TestStorageAPI:
    """Test cases for Storage API endpoints"""

    @allure.id("2715")
    @allure.title("GET /api/storage/status")
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
    @allure.title("GET /api/storage/list")
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
    @allure.title("POST /api/storage/mkdir")
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
        "POST /api/storage/write + GET /api/storage/read + DELETE /api/storage/remove"
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

    @allure.id("2675")
    @allure.title("POST /api/storage/write (file size limits)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_file_upload_size_limit(self, api_session, web_base_url):
        """Test file upload size limits"""

        test_file_path = "/ext/large_test_fil.bin"

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
