#pragma once
#include <furi.h>
#include <storage/storage_glue.h>
#include <storage/storage_sd_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STORAGE_FIRST_PARTITION_SIZE_MB (256)

void storage_ext_init(StorageData* storage, size_t logical_unit_number);

FS_Error storage_ext_init_bsp(void);
FS_Error storage_ext_mk_partititons(void);
FS_Error storage_ext_mount(StorageData* storage);
FS_Error storage_ext_unmount(StorageData* storage);

#ifdef __cplusplus
}
#endif
