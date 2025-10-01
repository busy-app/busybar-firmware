#include "check_update.h"
#include "parse_update_json.h"
#include <json_helper.h>
#include <toolbox/fetch/fetch_loader.h>
#include <furi_hal_version.h>
#include <toolbox/path.h>

#define TAG "CheckUpdate"

#define CHECK_UPDATE_JSON_FILE EXT_PATH("update/directory.json")

struct CheckUpdate {
    FuriThread* thread;
    FuriString* json_url;
    FuriString* channel_id;
    FuriString* current_firmware_version;
    FuriString* new_firmware_version;
    FuriString* new_firmware_url;
    FuriString* new_firmware_sha256;
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

    furi_check(furi_string_size(instance->channel_id) != 0);
    furi_check(furi_string_size(instance->json_url) != 0);
    check_update_get_current_version(instance, instance->current_firmware_version);

    //load directory.json
    FetchLoader* directory_json = fetch_loader_alloc();
    fetch_loader_run(
        directory_json, furi_string_get_cstr(instance->json_url), CHECK_UPDATE_JSON_FILE);
    while(!fetch_loader_is_processing_done(directory_json)) {
        furi_delay_ms(100);
    }
    fetch_loader_free(directory_json);

    ParseUpdateJson* parser = parse_update_init();
    if(parse_update_json(
           parser, CHECK_UPDATE_JSON_FILE, furi_string_get_cstr(instance->channel_id))) {
        if(strcmp(
               furi_string_get_cstr(instance->current_firmware_version),
               parse_update_get_version(parser)) != 0) {
            FURI_LOG_I(TAG, "New version available: %s", parse_update_get_version(parser));
            furi_string_set_str(instance->new_firmware_version, parse_update_get_version(parser));
            furi_string_set_str(instance->new_firmware_url, parse_update_get_url(parser));
            furi_string_set_str(instance->new_firmware_sha256, parse_update_get_sha256(parser));

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

CheckUpdate* check_update_init() {
    CheckUpdate* instance = malloc(sizeof(CheckUpdate));

    instance->thread = NULL;
    instance->json_url = furi_string_alloc();
    instance->channel_id = furi_string_alloc();
    instance->current_firmware_version = furi_string_alloc();
    instance->new_firmware_version = furi_string_alloc_printf("unknown");
    instance->new_firmware_url = furi_string_alloc();
    instance->new_firmware_sha256 = furi_string_alloc();
    instance->is_processing_semaphore = furi_semaphore_alloc(1, 1);
    return instance;
}

void check_update_free(CheckUpdate* instance) {
    furi_assert(instance);
    furi_check(!furi_semaphore_get_space(instance->is_processing_semaphore));
    furi_string_free(instance->json_url);
    furi_string_free(instance->channel_id);
    furi_string_free(instance->current_firmware_version);
    furi_string_free(instance->new_firmware_version);
    furi_string_free(instance->new_firmware_url);
    furi_string_free(instance->new_firmware_sha256);
    furi_semaphore_free(instance->is_processing_semaphore);
    free(instance);
}

void check_update_set_json_url(CheckUpdate* instance, const char* url) {
    furi_assert(instance);
    furi_string_set_str(instance->json_url, url);
}

void check_update_set_channel_id(CheckUpdate* instance, const char* channel_id) {
    furi_assert(instance);
    furi_check(channel_id);
    furi_string_set_str(instance->channel_id, channel_id);
}

void check_update_set_callback_done(
    CheckUpdate* instance,
    CheckUpdateCallbackDone callback,
    void* context) {
    furi_assert(instance);
    instance->callback_done = callback;
    instance->context = context;
}

void check_update_run(CheckUpdate* instance) {
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

bool check_update_is_new_version(CheckUpdate* instance) {
    furi_check(instance);

    return (
        furi_string_cmp(instance->current_firmware_version, instance->new_firmware_version) != 0);
}

void check_update_get_current_version(CheckUpdate* instance, FuriString* current_firmware_version) {
    furi_check(instance);
    UNUSED(current_firmware_version);
    const Version* firmware_version = furi_hal_version_get_firmware_version();
    if(strcmp(version_get_version(firmware_version), "unknown") == 0) {
        furi_string_set_str(current_firmware_version, version_get_githash(firmware_version));
    } else {
        furi_string_set_str(current_firmware_version, version_get_version(firmware_version));
    }
}

void check_update_get_new_version(CheckUpdate* instance, FuriString* new_firmware_version) {
    furi_check(instance);
    furi_string_set(new_firmware_version, instance->new_firmware_version);
}

void check_update_get_new_firmware_url(CheckUpdate* instance, FuriString* url) {
    furi_check(instance);
    furi_string_set(url, instance->new_firmware_url);
}

void check_update_get_new_firmware_sha256(CheckUpdate* instance, FuriString* sha256) {
    furi_check(instance);
    furi_string_set(sha256, instance->new_firmware_sha256);
}
