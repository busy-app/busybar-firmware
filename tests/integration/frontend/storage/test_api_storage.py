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

        try:
            storage_api.mkdir(test_dir)
        finally:
            storage_api.remove_raw(test_dir)

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
            remove_response = storage_api.remove_raw(test_file)
            assert remove_response.status_code == 200

    @allure.title("POST /api/storage/write (append=1 creates the file)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_write_append_creates_file(self, storage_api: StorageAPI):
        """Append to a non-existent file creates it"""
        test_file = f"/ext/test_append_create_{int(time.time())}.txt"
        content = b"Created by an append write"

        try:
            with allure.step(f"Verify file does not exist: {test_file}"):
                read_response = storage_api.read(test_file)
                assert read_response.status_code == 400, (
                    f"Expected 400 for a non-existent file, got {read_response.status_code}"
                )

            with allure.step("Append to the non-existent file"):
                write_response = storage_api.write(test_file, content, append=True)
                assert write_response.status_code == 200, (
                    f"Expected 200, got {write_response.status_code}"
                )

            with allure.step("Verify the file was created with the request body"):
                read_response = storage_api.read(test_file)
                assert read_response.status_code == 200, (
                    f"Expected 200, got {read_response.status_code}"
                )
                assert read_response.content == content, (
                    f"Expected {content!r}, got {read_response.content!r}"
                )

        finally:
            storage_api.remove_raw(test_file)

    @allure.title("POST /api/storage/write (append=1 appends to an existing file)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_write_append_extends_file(self, storage_api: StorageAPI):
        """Append adds the request body to the end of an existing file"""
        test_file = f"/ext/test_append_extend_{int(time.time())}.txt"
        first_part = b"First part of the file. "
        second_part = b"Second part of the file."

        try:
            with allure.step(f"Write initial content: {test_file}"):
                write_response = storage_api.write(test_file, first_part)
                assert write_response.status_code == 200, (
                    f"Expected 200, got {write_response.status_code}"
                )

            with allure.step("Append the second part"):
                write_response = storage_api.write(test_file, second_part, append=True)
                assert write_response.status_code == 200, (
                    f"Expected 200, got {write_response.status_code}"
                )

            with allure.step("Verify the file contains both parts in order"):
                read_response = storage_api.read(test_file)
                assert read_response.status_code == 200, (
                    f"Expected 200, got {read_response.status_code}"
                )
                assert read_response.content == first_part + second_part, (
                    f"Expected {(first_part + second_part)!r}, got {read_response.content!r}"
                )

        finally:
            storage_api.remove_raw(test_file)

    @allure.title("POST /api/storage/write (write without append replaces the file)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_write_replaces_file(self, storage_api: StorageAPI):
        """Write without the append parameter replaces the file content"""
        test_file = f"/ext/test_replace_{int(time.time())}.txt"
        old_content = b"Old content that must be replaced completely"
        # Shorter than the old content: appending or a partial
        # overwrite would leave the file longer than this.
        new_content = b"New content"

        try:
            with allure.step(f"Write initial content: {test_file}"):
                write_response = storage_api.write(test_file, old_content)
                assert write_response.status_code == 200, (
                    f"Expected 200, got {write_response.status_code}"
                )

            with allure.step("Write again without the append parameter"):
                write_response = storage_api.write(test_file, new_content)
                assert write_response.status_code == 200, (
                    f"Expected 200, got {write_response.status_code}"
                )

            with allure.step("Verify the file holds only the new content"):
                read_response = storage_api.read(test_file)
                assert read_response.status_code == 200, (
                    f"Expected 200, got {read_response.status_code}"
                )
                assert read_response.content == new_content, (
                    f"Expected {new_content!r}, got {read_response.content!r}"
                )

        finally:
            storage_api.remove_raw(test_file)

    @allure.title("POST /api/storage/write (append=0 replaces the file)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_write_append_zero_replaces_file(self, storage_api: StorageAPI):
        """An explicit append=0 replaces the file content, as documented"""
        test_file = f"/ext/test_append_zero_{int(time.time())}.txt"
        old_content = b"Old content that must be replaced completely"
        # Shorter than the old content: appending or a partial
        # overwrite would leave the file longer than this.
        new_content = b"New content"

        try:
            with allure.step(f"Write initial content: {test_file}"):
                write_response = storage_api.write(test_file, old_content)
                assert write_response.status_code == 200, (
                    f"Expected 200, got {write_response.status_code}"
                )

            with allure.step("Write again with an explicit append=0"):
                write_response = storage_api.write(test_file, new_content, append=False)
                assert write_response.status_code == 200, (
                    f"Expected 200, got {write_response.status_code}"
                )

            with allure.step("Verify the file holds only the new content"):
                read_response = storage_api.read(test_file)
                assert read_response.status_code == 200, (
                    f"Expected 200, got {read_response.status_code}"
                )
                assert read_response.content == new_content, (
                    f"Expected {new_content!r}, got {read_response.content!r}"
                )

        finally:
            storage_api.remove_raw(test_file)

    @allure.title("POST /api/storage/write (invalid append value)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.parametrize("append_value", ["2", "true"])
    def test_api_storage_write_append_invalid_value(
        self, storage_api: StorageAPI, append_value: str
    ):
        """An invalid append value is rejected and leaves the file unchanged"""
        test_file = f"/ext/test_append_invalid_{int(time.time())}.txt"
        content = b"Content that must survive a rejected write"

        try:
            with allure.step(f"Write initial content: {test_file}"):
                write_response = storage_api.write(test_file, content)
                assert write_response.status_code == 200, (
                    f"Expected 200, got {write_response.status_code}"
                )

            with allure.step(f"Write with append={append_value} is rejected"):
                response = storage_api.post_raw(
                    "/api/storage/write",
                    params={"path": test_file, "append": append_value},
                    data=b"rejected",
                    headers={"Content-Type": "application/octet-stream"},
                )
                assert response.status_code == 400, (
                    f"Expected 400, got {response.status_code}"
                )

            with allure.step("Verify the file content is unchanged"):
                read_response = storage_api.read(test_file)
                assert read_response.status_code == 200, (
                    f"Expected 200, got {read_response.status_code}"
                )
                assert read_response.content == content, (
                    f"Expected {content!r}, got {read_response.content!r}"
                )

        finally:
            storage_api.remove_raw(test_file)

    @allure.title("POST /api/storage/rename")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_rename(self, storage_api: StorageAPI):
        """Test POST /api/storage/rename endpoint"""
        original_path = f"/ext/test_rename_src_{int(time.time())}.txt"
        new_path = f"/ext/test_rename_dst_{int(time.time())}.txt"
        test_content = b"Rename test content"

        try:
            with allure.step(f"Write source file: {original_path}"):
                write_response = storage_api.write(original_path, test_content)
                assert write_response.status_code == 200

            with allure.step(f"Rename {original_path} -> {new_path}"):
                storage_api.rename(original_path, new_path)

            with allure.step("Verify renamed file is readable"):
                read_response = storage_api.read(new_path)
                assert read_response.status_code == 200
                assert read_response.content == test_content

            with allure.step("Verify original file no longer exists"):
                read_original = storage_api.read(original_path)
                assert read_original.status_code == 400

        finally:
            storage_api.remove_raw(original_path)
            storage_api.remove_raw(new_path)

    @allure.title("POST /api/storage/rename (non-existent source)")
    @pytest.mark.api
    @pytest.mark.frontend
    def test_api_storage_rename_nonexistent(self, storage_api: StorageAPI):
        """Test POST /api/storage/rename with non-existent source returns error"""
        response = storage_api.rename_raw(
            "/ext/nonexistent_file.txt",
            "/ext/new_name.txt",
        )
        assert response.status_code == 400

    @allure.id("2675")
    @allure.title("POST /api/storage/write (file size limits)")
    @pytest.mark.api
    @pytest.mark.frontend
    @pytest.mark.regression
    def test_api_file_upload_size_limit(self, storage_api: StorageAPI):
        """Test file upload size limits"""
        test_file_path = "/ext/large_test_fil.bin"
        large_content = b"x" * (10 * 1024 * 1024)  # 10MB

        with allure.step(f"Check if test file already exists: {test_file_path}"):
            check_response = storage_api.read(test_file_path)
            if check_response.status_code == 200:
                allure.attach(f"File {test_file_path} already exists, attempting cleanup", name="Pre-cleanup", attachment_type=allure.attachment_type.TEXT)
                delete_response = storage_api.remove_raw(test_file_path)
                if delete_response.status_code != 200:
                    pytest.fail(f"Failed to delete existing test file {test_file_path}")
                allure.attach("Successfully cleaned up existing file", name="Pre-cleanup Success", attachment_type=allure.attachment_type.TEXT)

        upload_successful = False

        try:
            with allure.step(f"Test large file upload to storage: {test_file_path}"):
                response = storage_api.write(test_file_path, large_content, timeout=120)

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
                    cleanup_response = storage_api.remove_raw(test_file_path)
                    if cleanup_response.status_code == 200:
                        allure.attach("Large test file cleaned up successfully", name="Cleanup Success", attachment_type=allure.attachment_type.TEXT)
                    else:
                        pytest.fail(f"Failed to clean up test file {test_file_path}")
