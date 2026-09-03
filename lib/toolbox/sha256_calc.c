#include "sha256_calc.h"

#include <storage/filesystem_api_defines.h>
#include <storage/storage.h>
#include <mbedtls/sha256.h>
#include <toolbox/hex.h>

#define SHA256_MAX_SIZE_CHUNK 1024 * 32 //32kb

bool sha256_calc_file(File* file, const char* path, unsigned char output[32], FS_Error* file_error) {
    if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        if(file_error != NULL) {
            *file_error = storage_file_get_error(file);
        }
        return false;
    }

    const size_t size_to_read = SHA256_MAX_SIZE_CHUNK;
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
    uint8_t raw_bytes[32];
    bool result = sha256_calc_file(file, path, raw_bytes, file_error);
    if(!result) return false;

    hex_bytes_to_string(raw_bytes, sizeof(raw_bytes), output);
    return true;
}

void sha256_calc_buffer(const uint8_t* buffer, size_t size, unsigned char output[32]) {
    furi_check(buffer);
    furi_check(output);

    mbedtls_sha256_context* sha256_ctx = malloc(sizeof(mbedtls_sha256_context));
    mbedtls_sha256_init(sha256_ctx);
    mbedtls_sha256_starts(sha256_ctx, 0);
    mbedtls_sha256_update(sha256_ctx, buffer, size);
    mbedtls_sha256_finish(sha256_ctx, output);
    free(sha256_ctx);
}

void sha256_string_calc_buffer(const uint8_t* buffer, size_t size, FuriString* output) {
    furi_check(buffer);
    furi_check(output);

    uint8_t raw_bytes[32];
    sha256_calc_buffer(buffer, size, raw_bytes);
    hex_bytes_to_string(raw_bytes, sizeof(raw_bytes), output);
}
