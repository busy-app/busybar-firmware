/**
 * @file tar_test.c
 * @brief Tar CLI command unit tests
 */

#include "../unit_tests.h"
#include <toolbox/path.h>
#include <toolbox/tar/tar_archive.h>
#include <applications/system/tar/tar.h>

#define TAG                       "TarTest"
#define TAR_UNIT_TESTS_PATH       UNIT_TESTS_PATH("tar_unit_test")
#define TAR_UNIT_TESTS_FILE_NAME1 TAR_UNIT_TESTS_PATH "/test1.txt"
#define TAR_UNIT_TESTS_FILE_NAME2 TAR_UNIT_TESTS_PATH "/test2.txt"
#define TAR_UNIT_TESTS_FILE_TAR   UNIT_TESTS_PATH("tar_test.tar")

#define TAR_UNIT_TESTS_EXTRACT_PATH       UNIT_TESTS_PATH("tar_unit_test_extract")
#define TAR_UNIT_TESTS_FILE_EXTRACT_NAME1 TAR_UNIT_TESTS_EXTRACT_PATH "/test1.txt"
#define TAR_UNIT_TESTS_FILE_EXTRACT_NAME2 TAR_UNIT_TESTS_EXTRACT_PATH "/test2.txt"

/*
 * The test tar file (test.tgz in assets) was created using python's tarfile module:
 *  python3 -m tarfile -c test.tar test.txt && gzip test.tar && mv test.tar.gz test.tgz
 *
 * Reason: microtar cannot handle tarballs created by the tar utility (at least on macos).
 * Octal values in tar headers are encoded differently:
 *  - macos tar inserts a space at the end: "000644 ",
 *  - python's tarfile zero-pads the number: "0000644".
 * Microtar fails when it encounters anything but an octal digit in the string.
 */
#define COMPRESSED_MESSAGE \
    "Flipper Zero is a tiny piece of hardware with a curious personality of a cyber-dolphin.\n"
#define GZIP_FILE         EXT_PATH("apps_assets/unit_tests/test.tgz")
#define GZIP_EXTRACT_PATH UNIT_TESTS_PATH("ungzip")

static void tar_config_setup(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file_handle1 = storage_file_alloc(storage);
    File* file_handle2 = storage_file_alloc(storage);
    FuriString* path = furi_string_alloc_printf("%s", TAR_UNIT_TESTS_PATH);

    do {
        if(path_recursive_create_dir(storage, path) != FSE_OK) {
            FURI_LOG_E(TAG, "Failed to create unit test directory: %s", TAR_UNIT_TESTS_PATH);
            break;
        }

        // Create test1.txt
        if(!storage_file_open(
               file_handle1, TAR_UNIT_TESTS_FILE_NAME1, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(TAG, "Failed to open file for writing: %s", TAR_UNIT_TESTS_FILE_NAME1);
            break;
        }

        for(size_t i = 0; i < 512; i++) {
            const char c = 'A' + (i % 26);
            if(storage_file_write(file_handle1, (uint8_t*)&c, 1) != 1) {
                FURI_LOG_E(TAG, "Failed to write data to file: %s", TAR_UNIT_TESTS_FILE_NAME1);
                break;
            }
        }

        storage_file_close(file_handle1);

        // Create test2.txt
        if(!storage_file_open(
               file_handle2, TAR_UNIT_TESTS_FILE_NAME2, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(TAG, "Failed to open file for writing: %s", TAR_UNIT_TESTS_FILE_NAME2);
            break;
        }

        for(size_t i = 0; i < 1024; i++) {
            const char c = 'A' + (i % 26);
            if(storage_file_write(file_handle2, (uint8_t*)&c, 1) != 1) {
                FURI_LOG_E(TAG, "Failed to write data to file: %s", TAR_UNIT_TESTS_FILE_NAME2);
                break;
            }
        }

        storage_file_close(file_handle2);

    } while(0);

    if(storage_file_is_open(file_handle1)) {
        storage_file_close(file_handle1);
    }
    if(storage_file_is_open(file_handle2)) {
        storage_file_close(file_handle2);
    }
    storage_file_free(file_handle1);
    storage_file_free(file_handle2);
    furi_record_close(RECORD_STORAGE);
    furi_string_free(path);
}

static void tar_config_teardown(void) {
}

MU_TEST(tar_test_cli) {
    FuriString* path = furi_string_alloc_printf("%s", TAR_UNIT_TESTS_FILE_TAR);
    FuriString* args = furi_string_alloc_printf("%s", TAR_UNIT_TESTS_PATH);
    mu_assert_int_eq(true, tar_compress_directory_cli(path, args));
    furi_string_set(args, TAR_UNIT_TESTS_EXTRACT_PATH);
    mu_assert_int_eq(true, tar_extract_files_cli(path, args));

    // File comparison
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file_handle1 = storage_file_alloc(storage);
    File* file_handle2 = storage_file_alloc(storage);

    do {
        // File comparison test1.txt
        if(!storage_file_open(
               file_handle1, TAR_UNIT_TESTS_FILE_NAME1, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Failed to open file for reading: %s", TAR_UNIT_TESTS_FILE_NAME1);
            break;
        }
        if(!storage_file_open(
               file_handle2, TAR_UNIT_TESTS_FILE_EXTRACT_NAME1, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(
                TAG, "Failed to open file for reading: %s", TAR_UNIT_TESTS_FILE_EXTRACT_NAME1);
            break;
        }

        int file1_size = storage_file_size(file_handle1);
        int file2_size = storage_file_size(file_handle2);
        mu_assert_int_eq(file1_size, file2_size);

        uint8_t buffer1[128];
        uint8_t buffer2[128];
        int total_read = 0;
        while(total_read < file1_size) {
            int to_read = MIN(128, file1_size - total_read);
            size_t read1 = storage_file_read(file_handle1, buffer1, (size_t)to_read);
            size_t read2 = storage_file_read(file_handle2, buffer2, (size_t)to_read);
            mu_assert_int_eq(read1, read2);
            mu_assert_int_eq(0, memcmp(buffer1, buffer2, read1));
            total_read += read1;
        }

        storage_file_close(file_handle1);
        storage_file_close(file_handle2);
        FURI_LOG_D(TAG, "File comparison test1.txt passed");

        // File comparison test2.txt
        if(!storage_file_open(
               file_handle1, TAR_UNIT_TESTS_FILE_NAME2, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Failed to open file for reading: %s", TAR_UNIT_TESTS_FILE_NAME2);
            break;
        }
        if(!storage_file_open(
               file_handle2, TAR_UNIT_TESTS_FILE_EXTRACT_NAME2, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(
                TAG, "Failed to open file for reading: %s", TAR_UNIT_TESTS_FILE_EXTRACT_NAME2);
            break;
        }
        file1_size = storage_file_size(file_handle1);
        file2_size = storage_file_size(file_handle2);
        mu_assert_int_eq(file1_size, file2_size);
        total_read = 0;
        while(total_read < file1_size) {
            int to_read = MIN(128, file1_size - total_read);
            size_t read1 = storage_file_read(file_handle1, buffer1, (size_t)to_read);
            size_t read2 = storage_file_read(file_handle2, buffer2, (size_t)to_read);
            mu_assert_int_eq(read1, read2);
            mu_assert_int_eq(0, memcmp(buffer1, buffer2, read1));
            total_read += read1;
        }
        storage_file_close(file_handle1);
        storage_file_close(file_handle2);
        FURI_LOG_D(TAG, "File comparison test2.txt passed");

    } while(0);

    if(storage_file_is_open(file_handle1)) {
        storage_file_close(file_handle1);
    }
    if(storage_file_is_open(file_handle2)) {
        storage_file_close(file_handle2);
    }
    storage_file_free(file_handle1);
    storage_file_free(file_handle2);

    furi_record_close(RECORD_STORAGE);
    furi_string_free(path);
    furi_string_free(args);
}

static bool check_file_contnents(Storage* storage, const char* path, const char* contents) {
    bool result = false;

    File* f = storage_file_alloc(storage);
    do {
        bool r = storage_file_open(f, path, FSAM_READ, FSOM_OPEN_EXISTING);
        if(!r) {
            mu_warn("Cannot open file");
            break;
        }

        uint64_t size = storage_file_size(f);
        size_t contents_len = strlen(contents);
        if(size != contents_len) {
            mu_warn("File size doesn't match");
            break;
        }

        char* buf = malloc(contents_len);
        size_t bytes_read = storage_file_read(f, buf, contents_len);
        if(bytes_read != contents_len) {
            mu_warn("Read length mismatch");
        } else {
            if(memcmp(buf, contents, contents_len) == 0) {
                result = true;
            } else {
                mu_warn("Contents mismatch");
            }
        }
        free(buf);
    } while(false);
    storage_file_free(f);
    return result;
}

MU_TEST(tar_test_gzip) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    TarArchive* tar = tar_archive_alloc(storage);

    {
        FuriString* gzip_extract_path = furi_string_alloc_set_str(GZIP_EXTRACT_PATH);
        FS_Error e = path_recursive_create_dir(storage, gzip_extract_path);
        mu_assert_int_eq(FSE_OK, e);
        furi_string_free(gzip_extract_path);
    }
    do {
        bool r = tar_archive_open(tar, GZIP_FILE, TarOpenModeReadAuto);
        mu_assert(r, "tar_archive_open");
        mu_assert_int_eq(1, tar_archive_get_entries_count(tar));
        r = tar_archive_unpack_to(tar, GZIP_EXTRACT_PATH, NULL);
        mu_assert(r, "tar_archive_unpack_to");
        check_file_contnents(storage, GZIP_EXTRACT_PATH "/test.txt", COMPRESSED_MESSAGE);
    } while(false);

    tar_archive_free(tar);
    furi_record_close(RECORD_STORAGE);
}

MU_TEST_SUITE(tar_test_suite) {
    MU_SUITE_CONFIGURE(&tar_config_setup, &tar_config_teardown);
    MU_RUN_TEST(tar_test_cli);
    MU_RUN_TEST(tar_test_gzip);
}

int run_minunit_tar_test(void) {
    MU_RUN_SUITE(tar_test_suite);
    return MU_EXIT_CODE;
}
