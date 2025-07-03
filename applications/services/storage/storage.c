#include <storage/storage.h>
#include <storage/storage_i.h>
#include <storage/storage_message.h>
#include <storage/storage_processing.h>
#include "storage/storage_glue.h"
#include "storages/storage_ext_sdmmc.h"
#include "storage_posix_api.h"

#define STORAGE_TICK 1000

#define TAG "Storage"

Storage* storage_app_alloc(void) {
    Storage* app = malloc(sizeof(Storage));
    app->message_queue = furi_message_queue_alloc(8, sizeof(StorageMessage));
    app->pubsub = furi_pubsub_alloc();

    for(uint8_t i = 0; i < STORAGE_COUNT; i++) {
        storage_data_init(&app->storage[i]);
        storage_data_timestamp(&app->storage[i]);
    }

    storage_ext_init(&app->storage[ST_EXT]);

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
