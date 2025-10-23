/**
 * @file tar_test.c
 * @brief Tar CLI command unit tests
 */

#include "../unit_tests.h"
#include <toolbox/path.h>
#include <applications/system/tar/tar.h>

#define TAG                       "TarTest"
#define TAR_UNIT_TESTS_PATH       UNIT_TESTS_PATH("tar_unit_test")
#define TAR_UNIT_TESTS_FILE_NAME1 TAR_UNIT_TESTS_PATH "/test1.txt"
#define TAR_UNIT_TESTS_FILE_NAME2 TAR_UNIT_TESTS_PATH "/test2.txt"
#define TAR_UNIT_TESTS_FILE_TAR   UNIT_TESTS_PATH("tar_test.tar")

#define TAR_UNIT_TESTS_EXTRACT_PATH       UNIT_TESTS_PATH("tar_unit_test_extract")
#define TAR_UNIT_TESTS_FILE_EXTRACT_NAME1 TAR_UNIT_TESTS_EXTRACT_PATH "/test1.txt"
#define TAR_UNIT_TESTS_FILE_EXTRACT_NAME2 TAR_UNIT_TESTS_EXTRACT_PATH "/test2.txt"

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

MU_TEST_SUITE(tar_test_suite) {
    MU_SUITE_CONFIGURE(&tar_config_setup, &tar_config_teardown);
    MU_RUN_TEST(tar_test_cli);
}

int run_minunit_tar_test(void) {
    MU_RUN_SUITE(tar_test_suite);
    return MU_EXIT_CODE;
}
