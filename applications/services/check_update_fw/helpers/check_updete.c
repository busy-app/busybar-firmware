#include "check_update.h"
#include "../check_update_fw.h"
#include "parse_update_json.h"
#include <furi.h>

#include <cjson/cJSON.h>
#include <storage/storage.h>

#define TAG "CheckUpdate"

#define CHECK_UPDATE_SETTINGS_FILE APP_DATA_PATH("settings.json")
#define CHECK_UPDATE_JSON_FILE     EXT_PATH("update/up.json")
#define CHECK_UPDATE_JSON_URL      "https://update.flipperzero.one/busybar-firmware/directory.json"

#define CHECK_UPDATE_BRANCH_ID_FOUND "development"

typedef enum {
    CheckUpdateStatusSuccess = (1UL << 1),
    CheckUpdateStatusError = (1UL << 2),
    CheckUpdateStatusDone = (1UL << 3),
} CheckUpdateStatus;

typedef struct {
    FuriString* url;
    FuriString* id;
    FuriString* version;
} CheckUpdate;

static int32_t check_update_thread_callback(void* context) {
    furi_assert(context);

    CheckUpdateFw* instance = context;

    CheckUpdate* update = malloc(sizeof(CheckUpdate));
    update->url = furi_string_alloc();
    update->id = furi_string_alloc();
    update->version = furi_string_alloc();

    CheckUpdateStatus status = 0;

    FURI_LOG_D(TAG, "Start");
    //TODO add downloading the file from the internet

    ParseUpdateJson* parser = parse_update_init();
    if(parse_update_json(parser, CHECK_UPDATE_JSON_FILE, CHECK_UPDATE_BRANCH_ID_FOUND)) {
        FURI_LOG_W(TAG, "Update ID found: %s", parse_update_get_id(parser));
        FURI_LOG_W(TAG, "Update version found: %s", parse_update_get_version(parser));
        FURI_LOG_W(TAG, "Update found: %s", parse_update_get_url(parser));
        FURI_LOG_W(TAG, "Update SHA256 found: %s", parse_update_get_sha256(parser));    
        status |= CheckUpdateStatusSuccess;
    } else {
        FURI_LOG_E(TAG, "Failed to parse update JSON");
        status |= CheckUpdateStatusError;
    }
    parse_update_free(parser);

    if(status & CheckUpdateStatusSuccess) {
        check_update_fw_status_update(instance, true);
    }

    FURI_LOG_D(TAG, "Stopping thread");

    free(update->url);
    free(update->id);
    free(update->version);
    free(update);
    return 0;
}

static void
    check_update_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    furi_assert(thread);
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
        FURI_LOG_D(TAG, "Stop");
    }
}

void check_update_startup(void* context) {
    furi_assert(context);
    FuriThread* startup_thread =
        furi_thread_alloc_ex("CheckUpdate", 1024 * 2, check_update_thread_callback, context);
    furi_thread_set_state_callback(startup_thread, check_update_thread_state_callback);
    FURI_LOG_D(TAG, "Starting thread");

    furi_thread_start(startup_thread);
}
