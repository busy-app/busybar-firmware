#pragma once
#include <furi.h>
#include <storage/storage_glue.h>
#include <storage/storage_sd_api.h>

#ifdef __cplusplus
extern "C" {
#endif

void storage_ext_init(StorageData* storage);
FS_Error sd_mount_card(StorageData* storage);
FS_Error sd_unmount_card(StorageData* storage);
FS_Error sd_format_card(StorageData* storage);
FS_Error sd_card_info(StorageData* storage, SDInfo* sd_info);
void sd_presence_changed(StorageData* storage);

#ifdef __cplusplus
}
#endif
