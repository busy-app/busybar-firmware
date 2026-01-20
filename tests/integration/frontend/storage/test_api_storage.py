import time

import allure
import pytest

from clients.api import StorageAPI


@allure.feature("5. Web Frontend")
@allure.story("Storage")
class TestStorageAPI:
    """Test cases for Storage API endpoints"""

    @allure.id("2679")
    @allure.title("GET /api/storage/status")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_status(self, storage_api: StorageAPI):
        """Test GET /api/storage/status endpoint"""
        response = storage_api.get_status()

        # Types validated by pydantic
        assert response.used_bytes >= 0
        assert response.free_bytes >= 0
        assert response.total_bytes >= 0

        # Validate logical consistency
        assert response.used_bytes + response.free_bytes <= response.total_bytes

    @allure.id("2648")
    @allure.title("GET /api/storage/list")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_list(self, storage_api: StorageAPI):
        """Test GET /api/storage/list endpoint"""
        response = storage_api.list(path="/ext")

        # Structure validated by pydantic
        assert isinstance(response.list, list)

        # Validate list items
        for item in response.list:
            assert item.type in ["file", "dir"]
            assert item.name
            if item.type == "file":
                assert item.size is not None

    @allure.id("2649")
    @allure.title("POST /api/storage/mkdir")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_mkdir(self, storage_api: StorageAPI):
        """Test POST /api/storage/mkdir endpoint"""
        test_dir = f"/ext/test_mkdir_{int(time.time())}"

        response = storage_api.mkdir(test_dir)

        assert response.result

    @allure.id("2650")
    @allure.title(
        "POST /api/storage/write + GET /api/storage/read + DELETE /api/storage/remove"
    )
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_write_read_remove(self, storage_api: StorageAPI):
        """Test storage write, read, and remove operations"""
        test_file = f"/ext/test_file_{int(time.time())}.txt"
        test_content = b"Test content for API storage test"

        with allure.step(f"Write test file: {test_file}"):
            write_response = storage_api.write(test_file, test_content)
            assert write_response.status_code == 200

        with allure.step(f"Read test file: {test_file}"):
            read_response = storage_api.read(test_file)
            assert read_response.status_code == 200
            assert read_response.content == test_content
            allure.attach(read_response.content.decode(), name="File Content", attachment_type=allure.attachment_type.TEXT)

        with allure.step(f"Remove test file: {test_file}"):
            remove_response = storage_api.remove(test_file)
            assert remove_response.status_code == 200

    @allure.id("2675")
    @allure.title("POST /api/storage/write (file size limits)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_file_upload_size_limit(self, storage_api: StorageAPI):
        """Test file upload size limits"""
        test_file_path = "/ext/large_test_fil.bin"
        large_content = b"x" * (10 * 1024 * 1024)  # 10MB

        with allure.step(f"Check if test file already exists: {test_file_path}"):
            check_response = storage_api.read(test_file_path)
            if check_response.status_code == 200:
                allure.attach(f"File {test_file_path} already exists, attempting cleanup", name="Pre-cleanup", attachment_type=allure.attachment_type.TEXT)
                delete_response = storage_api.remove(test_file_path)
                if delete_response.status_code != 200:
                    pytest.fail(f"Failed to delete existing test file {test_file_path}")
                allure.attach("Successfully cleaned up existing file", name="Pre-cleanup Success", attachment_type=allure.attachment_type.TEXT)

        upload_successful = False

        try:
            with allure.step(f"Test large file upload to storage: {test_file_path}"):
                response = storage_api.write(test_file_path, large_content, timeout=30)

            with allure.step("Verify size limit handling"):
                assert response.status_code in [200, 400, 413, 500]

                if response.status_code == 200:
                    upload_successful = True
                    allure.attach("Large file upload succeeded - no size limit enforced", name="Upload Result", attachment_type=allure.attachment_type.TEXT)

                    # Verify file was actually written
                    verify_response = storage_api.read(test_file_path)
                    if verify_response.status_code == 200:
                        actual_size = len(verify_response.content)
                        expected_size = len(large_content)
                        allure.attach(
                            f"File size verification: Expected {expected_size} bytes, got {actual_size} bytes",
                            name="Size Verification",
                            attachment_type=allure.attachment_type.TEXT
                        )
                        assert actual_size == expected_size
                else:
                    allure.attach(
                        f"Large file upload rejected with status {response.status_code}",
                        name="Upload Result",
                        attachment_type=allure.attachment_type.TEXT
                    )

        finally:
            if upload_successful:
                with allure.step(f"Clean up test file: {test_file_path}"):
                    cleanup_response = storage_api.remove(test_file_path)
                    if cleanup_response.status_code == 200:
                        allure.attach("Large test file cleaned up successfully", name="Cleanup Success", attachment_type=allure.attachment_type.TEXT)
                    else:
                        pytest.fail(f"Failed to clean up test file {test_file_path}")
