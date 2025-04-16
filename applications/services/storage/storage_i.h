#pragma once
#include <furi.h>
#include <furi_hal.h>

#include "storage_glue.h"
#include "storage_sd_api.h"
#include "filesystem_api_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STORAGE_COUNT (ST_INT + 1)

#define APPS_DATA_PATH   EXT_PATH("apps_data")
#define APPS_ASSETS_PATH EXT_PATH("apps_assets")

struct Storage {
    FuriMessageQueue* message_queue;
    StorageData storage[STORAGE_COUNT];
    FuriPubSub* pubsub;
#if(defined STM32U5G9xx) || (defined STM32U595xx) // FIXME:
    bool sd_alive;
#else
    StorageSDGui sd_gui;
#endif
};

#ifdef __cplusplus
}
#endif
