#pragma once

#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TempFile TempFile;

TempFile* temp_file_alloc(Storage* storage);

void temp_file_free(TempFile* instance);

bool temp_file_create(TempFile* instance, const char* path);

bool temp_file_write(TempFile* instance, const void* data, size_t data_len);

void temp_file_set_keep(TempFile* instance, bool keep);

#ifdef __cplusplus
}
#endif
