#include "check_update.h"
#include "../check_update_fw.h"
#include "parse_update_json.h"
#include <storage/storage.h>
#include <json_helper.h>
#include <toolbox/fetch/fetch_loader.h>

#define TAG "CheckUpdate"

#define CHECK_UPDATE_SETTINGS_FILE EXT_PATH("apps_data/check_update_fw/config.json")
#define CHECK_UPDATE_JSON_FILE     EXT_PATH("update/directory.json")

#define CHECK_UPDATE_JSON_URL_DEFAULT \
    "https://update.flipperzero.one/busybar-firmware/directory.json"
#define CHECK_UPDATE_JSON_CHANNEL_ID_DEFAULT "development"
#define CHECK_UPDATE_JSON_VERSION_DEFAULT    "unknown"

#define CHECK_UPDATE_JSON_URL_DIRECTORY       "url_directory_json"
#define CHECK_UPDATE_JSON_CURRENT_CHANNEL     "current_channel"
#define CHECK_UPDATE_JSON_CURRENT_VERSION     "current_version"
#define CHECK_UPDATE_JSON_NEW_VERSION         "new_version"
#define CHECK_UPDATE_JSON_NEW_FIRMWARE_URL    "new_firmware_url"
#define CHECK_UPDATE_JSON_NEW_FIRMWARE_SHA256 "new_firmware_sha256"

typedef enum {
    CheckUpdateStatusSuccess = (1UL << 1),
    CheckUpdateStatusError = (1UL << 2),
    CheckUpdateStatusDone = (1UL << 3),
    CheckUpdateStatusNoNewVersion = (1UL << 4),
    CheckUpdateStatusNewVersion = (1UL << 5),
} CheckUpdateStatus;

typedef struct {
    FuriString* url;
    FuriString* id;
    FuriString* version;
} CheckUpdate;

static int32_t check_update_thread_callback(void* context) {
    furi_assert(context);
    FURI_LOG_D(TAG, "Start");

    CheckUpdateFw* instance = context;
    UNUSED(instance);

    CheckUpdate* update = malloc(sizeof(CheckUpdate));
    update->url = furi_string_alloc();
    update->id = furi_string_alloc();
    update->version = furi_string_alloc();

    CheckUpdateStatus status = 0;

    // Load config
    if(json_config_read_single_str(
           CHECK_UPDATE_SETTINGS_FILE,
           CHECK_UPDATE_JSON_URL_DIRECTORY,
           update->url,
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
           update->id,
           CHECK_UPDATE_JSON_CHANNEL_ID_DEFAULT) == JsonConfigStatusMissing) {
        FURI_LOG_W(TAG, "No channel ID found, using default");
        json_config_write_single_str(
            CHECK_UPDATE_SETTINGS_FILE,
            CHECK_UPDATE_JSON_CURRENT_CHANNEL,
            CHECK_UPDATE_JSON_CHANNEL_ID_DEFAULT);
    }
    if(json_config_read_single_str(
           CHECK_UPDATE_SETTINGS_FILE,
           CHECK_UPDATE_JSON_CURRENT_VERSION,
           update->version,
           CHECK_UPDATE_JSON_VERSION_DEFAULT) == JsonConfigStatusMissing) {
        FURI_LOG_W(TAG, "No current version found, using default");
        json_config_write_single_str(
            CHECK_UPDATE_SETTINGS_FILE,
            CHECK_UPDATE_JSON_CURRENT_VERSION,
            CHECK_UPDATE_JSON_VERSION_DEFAULT);
    }

    //load directory.json
    FetchLoader* directory_json = fetch_loader_alloc();
    fetch_loader_run(directory_json, furi_string_get_cstr(update->url), CHECK_UPDATE_JSON_FILE);
    while(!fetch_loader_is_processing_done(directory_json)) {
        furi_delay_ms(100);
    }
    fetch_loader_free(directory_json);

    ParseUpdateJson* parser = parse_update_init();
    if(parse_update_json(parser, CHECK_UPDATE_JSON_FILE, furi_string_get_cstr(update->id))) {
        if(strcmp(furi_string_get_cstr(update->version), parse_update_get_version(parser)) != 0) {
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

            status |= CheckUpdateStatusNewVersion;
        } else {
            FURI_LOG_I(TAG, "No new version available");
            status |= CheckUpdateStatusNoNewVersion;
        }

    } else {
        FURI_LOG_E(TAG, "Failed to parse update JSON");
        status |= CheckUpdateStatusError;
    }

    parse_update_free(parser);

    // TODO: add status processing
    UNUSED(status);
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
    CheckUpdateFw* instance = context;

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
        FURI_LOG_D(TAG, "Stop");
        check_update_fw_status_update(instance, true);
    }
}

void check_update_startup(void* context) {
    furi_assert(context);
    FuriThread* startup_thread =
        furi_thread_alloc_ex("CheckUpdate", 1024 * 2, check_update_thread_callback, context);
    furi_thread_set_state_context(startup_thread, context);
    furi_thread_set_state_callback(startup_thread, check_update_thread_state_callback);
    FURI_LOG_D(TAG, "Starting thread");

    furi_thread_start(startup_thread);
}

bool check_update_is_new_version(void) {
    FuriString* current_version = furi_string_alloc();
    FuriString* new_version = furi_string_alloc();

    json_config_read_single_str(
        CHECK_UPDATE_SETTINGS_FILE,
        CHECK_UPDATE_JSON_CURRENT_VERSION,
        current_version,
        CHECK_UPDATE_JSON_VERSION_DEFAULT);
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
    json_config_read_single_str(
        CHECK_UPDATE_SETTINGS_FILE,
        CHECK_UPDATE_JSON_CURRENT_VERSION,
        current_version,
        CHECK_UPDATE_JSON_VERSION_DEFAULT);
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

void check_update_set_current_version(const char* version) {
    json_config_write_single_str(
        CHECK_UPDATE_SETTINGS_FILE, CHECK_UPDATE_JSON_CURRENT_VERSION, version);
}
