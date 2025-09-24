#include "sha256_calc.h"

#include <storage/filesystem_api_defines.h>
#include <storage/storage.h>
#include <mbedtls/sha256.h>

bool sha256_calc_file(File* file, const char* path, unsigned char output[32], FS_Error* file_error) {
    if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        if(file_error != NULL) {
            *file_error = storage_file_get_error(file);
        }
        return false;
    }

    const size_t size_to_read = 512;
    uint8_t* data = malloc(size_to_read);
    bool result = true;

    mbedtls_sha256_context* sha256_ctx = malloc(sizeof(mbedtls_sha256_context));
    mbedtls_sha256_init(sha256_ctx);
    mbedtls_sha256_starts(sha256_ctx, 0);
    while(true) {
        size_t read_size = storage_file_read(file, data, size_to_read);
        if(storage_file_get_error(file) != FSE_OK) {
            result = false;
            break;
        }
        if(read_size == 0) {
            break;
        }
        mbedtls_sha256_update(sha256_ctx, data, read_size);
    }
    mbedtls_sha256_finish(sha256_ctx, output);
    free(sha256_ctx);
    free(data);

    if(file_error != NULL) {
        *file_error = storage_file_get_error(file);
    }

    storage_file_close(file);
    return result;
}

bool sha256_string_calc_file(
    File* file,
    const char* path,
    FuriString* output,
    FS_Error* file_error) {
    const size_t hash_size = 32;
    unsigned char hash[hash_size];
    bool result = sha256_calc_file(file, path, hash, file_error);

    if(result) {
        furi_string_set(output, "");
        for(size_t i = 0; i < hash_size; i++) {
            furi_string_cat_printf(output, "%02x", hash[i]);
        }
    }

    return result;
}
