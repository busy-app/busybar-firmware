#pragma once

#include <stdint.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

bool sha256_calc_file(File* file, const char* path, unsigned char output[32], FS_Error* file_error);

bool sha256_string_calc_file(
    File* file,
    const char* path,
    FuriString* output,
    FS_Error* file_error);

void sha256_calc_buffer(const uint8_t* buffer, size_t size, uint8_t output[32]);

void sha256_string_calc_buffer(const uint8_t* buffer, size_t size, FuriString* output);

#ifdef __cplusplus
}
#endif
