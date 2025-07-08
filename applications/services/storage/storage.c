#include <storage/storage.h>
#include <storage/storage_i.h>
#include <storage/storage_message.h>
#include <storage/storage_processing.h>
#include "storage/storage_glue.h"
#include "storages/storage_ext_sdmmc.h"
#include "storage_posix_api.h"

#define STORAGE_TICK 1000

#define TAG "Storage"

#include <furi_hal.h>

extern FS_Error storage_process_common_fs_info(
    Storage* app,
    FuriString* path,
    uint64_t* total_space,
    uint64_t* free_space,
    bool* is_read_only);

Storage* storage_app_alloc(void) {
    Storage* app = malloc(sizeof(Storage));
    app->message_queue = furi_message_queue_alloc(8, sizeof(StorageMessage));
    app->pubsub = furi_pubsub_alloc();
    app->path_aliased = furi_string_alloc();
    app->path_storage = furi_string_alloc();
    furi_string_reserve(app->path_aliased, 256);
    furi_string_reserve(app->path_storage, 256);

    for(uint8_t i = 0; i < STORAGE_COUNT; i++) {
        storage_data_init(&app->storage[i]);
        storage_data_timestamp(&app->storage[i]);
    }

    StorageData* storage_bkp = &app->storage[ST_BKP];
    StorageData* storage_ext = &app->storage[ST_EXT];

    storage_ext_init(storage_bkp, ST_BKP);
    storage_ext_init(storage_ext, ST_EXT);

    storage_set_read_only(storage_bkp, false);
    storage_set_read_only(storage_ext, false);

    // mount storages
    do {
        FS_Error ret = storage_ext_init_bsp();
        if(ret != FSE_OK) {
            FURI_LOG_E(TAG, "Storage bsp init failed: %d", ret);
            break;
        }

        // // Uncomment this to remove partitions information
        // {
        //     uint8_t buffer[1024] = {0};
        //     furi_hal_sdmmc_write_blocks(
        //         buffer, 0, 2, 10000); // dummy write to remove old partitions

        //     furi_crash("remove me");
        // }

        {
            ret = storage_ext_mount(storage_bkp);
            if(ret != FSE_OK) {
                FURI_LOG_E(TAG, "Backup mount failed: %s", storage_data_status_text(storage_bkp));
                break;
            }

            // check if backup partition is valid in terms of size
            {
                uint64_t total_space;
                uint64_t free_space;
                bool is_read_only;

                FuriString* path = furi_string_alloc_set(STORAGE_BACKUP_PATH_PREFIX);
                FS_Error error = storage_process_common_fs_info(
                    app, path, &total_space, &free_space, &is_read_only);
                furi_string_free(path);

                if(error != FSE_OK) {
                    FURI_LOG_E(
                        TAG, "Backup check failed: %s", storage_data_status_text(storage_bkp));
                    storage_ext_unmount(storage_bkp);
                    break;
                }

                if(total_space / (1024 * 1024) > (STORAGE_FIRST_PARTITION_SIZE_MB * 2)) {
                    FURI_LOG_E(
                        TAG,
                        "Backup partition is too big, probably old partition table, size: %lluMiB",
                        total_space / (1024 * 1024));
                    storage_ext_unmount(storage_bkp);
                    break;
                }
            }

            // Set backup storage read-only after mount
            storage_set_read_only(storage_bkp, true);
        }

        {
            ret = storage_ext_mount(storage_ext);
            if(ret != FSE_OK) {
                FURI_LOG_E(
                    TAG, "External mount failed: %s", storage_data_status_text(storage_ext));
                break;
            }
        }
    } while(false);

    storage_posix_api_init(app);

    return app;
}

int32_t storage_srv(void* p) {
    UNUSED(p);
    Storage* app = storage_app_alloc();
    furi_record_create(RECORD_STORAGE, app);

    StorageMessage message;
    while(1) {
        if(furi_message_queue_get(app->message_queue, &message, FuriWaitForever) == FuriStatusOk) {
            storage_process_message(app, &message);
        }
    }

    return 0;
}
