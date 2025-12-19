import time

import allure
import pytest

from utils import (
    api_get,
    api_post,
    api_delete,
    attach_text,
    attach_json,
    assert_has_fields,
    assert_field_type,
    assert_field_in,
)


@allure.feature("5. Web Frontend")
@allure.story("Storage")
class TestStorageAPI:
    """Test cases for Storage API endpoints"""

    @allure.id("2679")
    @allure.title("GET /api/storage/status")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_status(self, api_session, web_base_url):
        """Test GET /api/storage/status endpoint"""

        with allure.step("Make GET request to /api/storage/status"):
            response = api_get(api_session, web_base_url, "/api/storage/status")

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields(
                "used_bytes", "free_bytes", "total_bytes"
            ).attach_to_allure("Storage Status Response")

            # Validate types
            data = response.json()
            for field in ["used_bytes", "free_bytes", "total_bytes"]:
                assert_field_type(data, field, int)

            # Validate logical consistency
            total = data["total_bytes"]
            used = data["used_bytes"]
            free = data["free_bytes"]
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
            response = api_get(
                api_session, web_base_url, "/api/storage/list",
                params={"path": "/ext"}
            )

        with allure.step("Verify response status and structure"):
            response.assert_ok().assert_json_content_type()
            response.assert_has_fields("list").attach_to_allure("Storage List Response")

            data = response.json()
            assert_field_type(data, "list", list)

            # Validate list items if any exist
            for item in data["list"]:
                assert_has_fields(item, "type", "name")
                assert_field_in(item, "type", ["file", "dir"])

                if item["type"] == "file":
                    assert_has_fields(item, "size")
                    assert_field_type(item, "size", int)

    @allure.id("2649")
    @allure.title("POST /api/storage/mkdir")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_mkdir(self, api_session, web_base_url):
        """Test POST /api/storage/mkdir endpoint"""

        test_dir = "/ext/test_mkdir_" + str(int(time.time()))

        with allure.step(f"Create test directory: {test_dir}"):
            response = api_post(
                api_session, web_base_url, "/api/storage/mkdir",
                params={"path": test_dir}
            )

        with allure.step("Verify directory creation response"):
            response.assert_ok()
            response.assert_has_fields("result").attach_to_allure("Mkdir Response")

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
            response = api_post(
                api_session, web_base_url, "/api/storage/write",
                params={"path": test_file},
                data=test_content,
                headers={"Content-Type": "application/octet-stream"},
            )
            response.assert_ok()
            response.attach_to_allure("Write Response")

            with allure.step(f"Read test file: {test_file}"):
                read_response = api_get(
                    api_session, web_base_url, "/api/storage/read",
                    params={"path": test_file}
                )
                read_response.assert_ok()
                assert (
                    read_response.response.content == test_content
                ), "Read content should match written content"
                attach_text(read_response.response.content.decode(), "File Content")

                with allure.step(f"Remove test file: {test_file}"):
                    remove_response = api_delete(
                        api_session, web_base_url, "/api/storage/remove",
                        params={"path": test_file}
                    )
                    remove_response.assert_ok()
                    remove_response.attach_to_allure("Remove Response")

    @allure.id("2675")
    @allure.title("POST /api/storage/write (file size limits)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_file_upload_size_limit(self, api_session, web_base_url):
        """Test file upload size limits"""

        test_file_path = "/ext/large_test_fil.bin"
        large_content = b"x" * (10 * 1024 * 1024)  # 10MB

        with allure.step(f"Check if test file already exists: {test_file_path}"):
            check_response = api_get(
                api_session, web_base_url, "/api/storage/read",
                params={"path": test_file_path}
            )
            if check_response.status_code == 200:
                attach_text(
                    f"File {test_file_path} already exists, attempting cleanup",
                    "Pre-cleanup"
                )
                delete_response = api_delete(
                    api_session, web_base_url, "/api/storage/remove",
                    params={"path": test_file_path}
                )
                if delete_response.status_code != 200:
                    pytest.fail(
                        f"Failed to delete existing test file {test_file_path}. Status: {delete_response.status_code}"
                    )
                else:
                    attach_text(
                        f"Successfully cleaned up existing file {test_file_path}",
                        "Pre-cleanup Success"
                    )

        upload_successful = False

        try:
            with allure.step(f"Test large file upload to storage: {test_file_path}"):
                response = api_post(
                    api_session, web_base_url, "/api/storage/write",
                    params={"path": test_file_path},
                    data=large_content,
                    headers={"Content-Type": "application/octet-stream"},
                    timeout=30,
                )

            with allure.step("Verify size limit handling"):
                response.assert_status([200, 400, 413, 500])

                if response.status_code == 200:
                    upload_successful = True
                    attach_text(
                        "Large file upload succeeded - no size limit enforced",
                        "Upload Result"
                    )

                    # Verify file was actually written
                    verify_response = api_get(
                        api_session, web_base_url, "/api/storage/read",
                        params={"path": test_file_path}
                    )
                    if verify_response.status_code == 200:
                        actual_size = len(verify_response.response.content)
                        expected_size = len(large_content)
                        attach_text(
                            f"File size verification: Expected {expected_size} bytes, got {actual_size} bytes",
                            "Size Verification"
                        )
                        assert (
                            actual_size == expected_size
                        ), f"File size mismatch: expected {expected_size}, got {actual_size}"
                else:
                    attach_text(
                        f"Large file upload rejected with status {response.status_code} - size limits enforced",
                        "Upload Result"
                    )

        finally:
            if upload_successful:
                with allure.step(f"Clean up test file: {test_file_path}"):
                    cleanup_response = api_delete(
                        api_session, web_base_url, "/api/storage/remove",
                        params={"path": test_file_path}
                    )

                    if cleanup_response.status_code == 200:
                        attach_text(
                            "Large test file cleaned up successfully",
                            "Cleanup Success"
                        )
                    else:
                        error_msg = f"Failed to clean up test file {test_file_path}. Status: {cleanup_response.status_code}"
                        attach_text(error_msg, "Cleanup Failure")
                        pytest.fail(error_msg)
