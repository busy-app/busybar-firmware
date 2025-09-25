#include "check_update.h"
#include "../check_update_fw.h"
#include "parse_update_json.h"
#include <storage/storage.h>
#include <json_helper.h>
#include <toolbox/fetch/fetch_loader.h>
#include <furi_hal_version.h>
#include <toolbox/path.h>

#define TAG "CheckUpdate"

#define CHECK_UPDATE_SETTINGS_FILE EXT_PATH("apps_data/check_update_fw/config.json")
#define CHECK_UPDATE_JSON_FILE     EXT_PATH("update/directory.json")

#define CHECK_UPDATE_JSON_URL_DEFAULT \
    "https://update.flipperzero.one/busybar-firmware/directory.json"
#define CHECK_UPDATE_JSON_CHANNEL_ID_DEFAULT "development"
#define CHECK_UPDATE_JSON_VERSION_DEFAULT    "unknown"

#define CHECK_UPDATE_JSON_URL_DIRECTORY       "url_directory_json"
#define CHECK_UPDATE_JSON_CURRENT_CHANNEL     "current_channel"
#define CHECK_UPDATE_JSON_NEW_VERSION         "new_version"
#define CHECK_UPDATE_JSON_NEW_FIRMWARE_URL    "new_firmware_url"
#define CHECK_UPDATE_JSON_NEW_FIRMWARE_SHA256 "new_firmware_sha256"

// typedef enum {
//     CheckUpdateStatusSuccess = (1UL << 1),
//     CheckUpdateStatusError = (1UL << 2),
//     CheckUpdateStatusDone = (1UL << 3),
//     CheckUpdateStatusNoNewVersion = (1UL << 4),
//     CheckUpdateStatusNewVersion = (1UL << 5),
// } CheckUpdateStatus;

struct CheckUpdate {
    FuriThread* thread;
    FuriString* url;
    FuriString* id;
    FuriString* version;
    CheckUpdateStatus status;
    FuriSemaphore* is_processing_semaphore;
    CheckUpdateCallbackDone callback_done;
    void* context;
};

static int32_t check_update_thread_callback(void* context) {
    furi_assert(context);
    FURI_LOG_D(TAG, "Start");
    CheckUpdate* instance = context;
    instance->status = 0;

    // Load config
    if(json_config_read_single_str(
           CHECK_UPDATE_SETTINGS_FILE,
           CHECK_UPDATE_JSON_URL_DIRECTORY,
           instance->url,
           CHECK_UPDATE_JSON_URL_DEFAULT) == JsonConfigStatusMissing) {
        FURI_LOG_W(TAG, "No URL found, using default");
        json_config_write_single_str(
            CHECK_UPDATE_SETTINGS_FILE,
            CHECK_UPDATE_JSON_URL_DIRECTORY,
            CHECK_UPDATE_JSON_URL_DEFAULT);
    }
    if(json_config_read_single_str(
           CHECK_UPDATE_SETTINGS_FILE,
           CHECK_UPDATE_JSON_CURRENT_CHANNEL,
           instance->id,
           CHECK_UPDATE_JSON_CHANNEL_ID_DEFAULT) == JsonConfigStatusMissing) {
        FURI_LOG_W(TAG, "No channel ID found, using default");
        json_config_write_single_str(
            CHECK_UPDATE_SETTINGS_FILE,
            CHECK_UPDATE_JSON_CURRENT_CHANNEL,
            CHECK_UPDATE_JSON_CHANNEL_ID_DEFAULT);
    }
    check_update_get_current_version(instance->version);

    //load directory.json
    FetchLoader* directory_json = fetch_loader_alloc();
    fetch_loader_run(directory_json, furi_string_get_cstr(instance->url), CHECK_UPDATE_JSON_FILE);
    while(!fetch_loader_is_processing_done(directory_json)) {
        furi_delay_ms(100);
    }
    fetch_loader_free(directory_json);

    ParseUpdateJson* parser = parse_update_init();
    if(parse_update_json(parser, CHECK_UPDATE_JSON_FILE, furi_string_get_cstr(instance->id))) {
        if(strcmp(furi_string_get_cstr(instance->version), parse_update_get_version(parser)) !=
           0) {
            FURI_LOG_I(TAG, "New version available: %s", parse_update_get_version(parser));
            json_config_write_single_str(
                CHECK_UPDATE_SETTINGS_FILE,
                CHECK_UPDATE_JSON_NEW_VERSION,
                parse_update_get_version(parser));

            json_config_write_single_str(
                CHECK_UPDATE_SETTINGS_FILE,
                CHECK_UPDATE_JSON_NEW_FIRMWARE_URL,
                parse_update_get_url(parser));

            json_config_write_single_str(
                CHECK_UPDATE_SETTINGS_FILE,
                CHECK_UPDATE_JSON_NEW_FIRMWARE_SHA256,
                parse_update_get_sha256(parser));

            instance->status |= CheckUpdateStatusNewVersion;
        } else {
            FURI_LOG_I(TAG, "No new version available");
            instance->status |= CheckUpdateStatusNoNewVersion;
        }
        instance->status |= CheckUpdateStatusSuccess;
    } else {
        FURI_LOG_E(TAG, "Failed to parse update JSON");
        instance->status |= CheckUpdateStatusError;
    }

    parse_update_free(parser);

    FURI_LOG_D(TAG, "Stopping thread");
    return 0;
}

static void
    check_update_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    furi_assert(thread);
    CheckUpdate* instance = context;

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
        furi_semaphore_release(instance->is_processing_semaphore);
        instance->thread = NULL;
        instance->status |= CheckUpdateStatusDone;
        if(instance->callback_done) {
            instance->callback_done(instance->status, instance->context);
        }
        FURI_LOG_D(TAG, "Stop");
    }
}

static void check_update_checking_folder(CheckUpdate* instance) {
    UNUSED(instance);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* path = furi_string_alloc();
    path_extract_dirname(CHECK_UPDATE_SETTINGS_FILE, path);

    if(path_recursive_create_dir(storage, path) != FSE_OK) {
        FURI_LOG_E(TAG, "Failed to create directory: %s", furi_string_get_cstr(path));
    }

    furi_string_free(path);
    furi_record_close(RECORD_STORAGE);
}

CheckUpdate* check_update_init() {
    CheckUpdate* instance = malloc(sizeof(CheckUpdate));

    instance->thread = NULL;
    instance->url = furi_string_alloc();
    instance->id = furi_string_alloc();
    instance->version = furi_string_alloc();
    instance->is_processing_semaphore = furi_semaphore_alloc(1, 1);

    check_update_checking_folder(instance);
    return instance;
}

void check_update_free(CheckUpdate* instance) {
    furi_assert(instance);
    furi_check(!furi_semaphore_get_space(instance->is_processing_semaphore));
    furi_string_free(instance->url);
    furi_string_free(instance->id);
    furi_string_free(instance->version);
    furi_semaphore_free(instance->is_processing_semaphore);
    free(instance);
}

void check_update_set_callback_done(
    CheckUpdate* instance,
    CheckUpdateCallbackDone callback,
    void* context) {
    furi_assert(instance);
    instance->callback_done = callback;
    instance->context = context;
}

void check_update_startup(CheckUpdate* instance) {
    furi_assert(instance);
    furi_check(!furi_semaphore_get_space(instance->is_processing_semaphore));
    furi_semaphore_acquire(instance->is_processing_semaphore, FuriWaitForever);
    instance->thread =
        furi_thread_alloc_ex("CheckUpdate", 1024 * 2, check_update_thread_callback, instance);
    furi_thread_set_state_context(instance->thread, instance);
    furi_thread_set_state_callback(instance->thread, check_update_thread_state_callback);
    FURI_LOG_D(TAG, "Starting thread");

    furi_thread_start(instance->thread);
}

bool check_update_is_processing_done(CheckUpdate* instance) {
    furi_check(instance);
    return !furi_semaphore_get_space(instance->is_processing_semaphore);
}

bool check_update_is_new_version(void) {
    FuriString* current_version = furi_string_alloc();
    FuriString* new_version = furi_string_alloc();

    check_update_get_current_version(current_version);
    json_config_read_single_str(
        CHECK_UPDATE_SETTINGS_FILE,
        CHECK_UPDATE_JSON_NEW_VERSION,
        new_version,
        CHECK_UPDATE_JSON_VERSION_DEFAULT);

    bool ret = (furi_string_cmp(current_version, new_version) != 0);

    furi_string_free(current_version);
    furi_string_free(new_version);
    return ret;
}

void check_update_get_current_version(FuriString* current_version) {
    const Version* firmware_version = furi_hal_version_get_firmware_version();
    if(strcmp(version_get_version(firmware_version), "unknown") == 0) {
        furi_string_set_str(current_version, version_get_githash(firmware_version));
    } else {
        furi_string_set_str(current_version, version_get_version(firmware_version));
    }
}

void check_update_get_new_version(FuriString* new_version) {
    json_config_read_single_str(
        CHECK_UPDATE_SETTINGS_FILE,
        CHECK_UPDATE_JSON_NEW_VERSION,
        new_version,
        CHECK_UPDATE_JSON_VERSION_DEFAULT);
}

void check_update_get_new_firmware_url(FuriString* url) {
    json_config_read_single_str(
        CHECK_UPDATE_SETTINGS_FILE, CHECK_UPDATE_JSON_NEW_FIRMWARE_URL, url, "");
}

void check_update_get_new_firmware_sha256(FuriString* sha256) {
    json_config_read_single_str(
        CHECK_UPDATE_SETTINGS_FILE, CHECK_UPDATE_JSON_NEW_FIRMWARE_SHA256, sha256, "");
}
