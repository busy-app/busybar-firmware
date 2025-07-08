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

    storage_ext_init(&app->storage[ST_BKP], ST_BKP);
    storage_ext_init(&app->storage[ST_EXT], ST_EXT);

    storage_set_read_only(&app->storage[ST_BKP], false);
    storage_set_read_only(&app->storage[ST_EXT], false);

    // mount storages
    do {
        FS_Error ret = storage_ext_init_bsp();
        if(ret != FSE_OK) {
            FURI_LOG_E(TAG, "Storage bsp init failed: %d", ret);
            break;
        }

        // Uncomment this to remove partitions information
        // {
        //     uint8_t buffer[1024] = {0};
        //     furi_hal_sdmmc_write_blocks(
        //         buffer, 0, 2, 10000); // dummy write to remove old partitions

        //     furi_crash("remove me");
        // }

        ret = storage_ext_mount(&app->storage[ST_BKP]);
        if(ret != FSE_OK) {
            FURI_LOG_E(
                TAG, "Storage mount failed: %s", storage_data_status_text(&app->storage[ST_BKP]));
            break;
        }

        // Set backup storage read-only after mount
        storage_set_read_only(&app->storage[ST_BKP], true);

        ret = storage_ext_mount(&app->storage[ST_EXT]);
        if(ret != FSE_OK) {
            FURI_LOG_E(
                TAG, "Storage mount failed: %s", storage_data_status_text(&app->storage[ST_EXT]));
            break;
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
