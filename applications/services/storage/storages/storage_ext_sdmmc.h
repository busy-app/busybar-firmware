#pragma once
#include <furi.h>
#include <storage/storage_glue.h>
#include <storage/storage_sd_api.h>

#ifdef __cplusplus
extern "C" {
#endif

void storage_ext_init(StorageData* storage);

#ifdef __cplusplus
}
#endif
