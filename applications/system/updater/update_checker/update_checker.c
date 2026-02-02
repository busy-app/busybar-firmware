#include "update_checker.h"
#include "../updater_paths.h"
#include "../helpers/update_parser.h"

#include <toolbox/fetch/fetch_loader.h>

#define TAG "UpdateChecker"

#define THREAD_NAME       "UpdateChecker"
#define THREAD_STACK_SIZE (2 * 1024)

typedef enum {
    ThreadFlagDirectoryLoadSuccess = 1 << 0,
    ThreadFlagDirectoryLoadFailure = 1 << 1,
} ThreadFlag;

struct UpdateChecker {
    FuriThread* thread;
    FuriSemaphore* run_lock;

    FuriString* url;
    FuriString* channel_id;

    UpdateMetadata update_metadata;
    bool was_check_successful;

    UpdateCheckerDoneCallback done_callback;
    void* done_callback_context;
};

static void directory_load_done_callback(FetchLoaderDoneStatus done_status, void* context) {
    UpdateChecker* instance = context;

    furi_thread_flags_set(
        furi_thread_get_id(instance->thread),
        (done_status == FetchLoaderDoneStatusSuccess) ? ThreadFlagDirectoryLoadSuccess :
                                                        ThreadFlagDirectoryLoadFailure);
}

static int32_t thread_callback(void* context) {
    UpdateChecker* instance = context;
    const char* url = furi_string_get_cstr(instance->url);

    FetchLoader* directory_loader = fetch_loader_alloc();
    fetch_loader_set_done_callback(directory_loader, directory_load_done_callback, instance);

    FURI_LOG_D(
        TAG, "Downloading directory file from %s to %s...", url, UPDATER_CHECK_DIRECTORY_PATH);
    fetch_loader_run(directory_loader, url, UPDATER_CHECK_DIRECTORY_PATH);

    uint32_t flags = furi_thread_flags_wait(
        ThreadFlagDirectoryLoadSuccess | ThreadFlagDirectoryLoadFailure,
        FuriFlagWaitAny,
        FuriWaitForever);

    fetch_loader_free(directory_loader);

    do {
        if(flags & ThreadFlagDirectoryLoadFailure) {
            instance->was_check_successful = false;
            FURI_LOG_E(
                TAG,
                "Failed to download directory file from %s to %s",
                url,
                UPDATER_CHECK_DIRECTORY_PATH);
            break;
        }

        FURI_LOG_D(TAG, "Parsing directory file %s...", UPDATER_CHECK_DIRECTORY_PATH);
        bool is_parse_success = update_parser_metadata_parse(
            UPDATER_CHECK_DIRECTORY_PATH,
            furi_string_get_cstr(instance->channel_id),
            &instance->update_metadata);

        if(!is_parse_success) {
            instance->was_check_successful = false;
            FURI_LOG_E(TAG, "Failed to parse directory file %s", UPDATER_CHECK_DIRECTORY_PATH);
            break;
        }

        instance->was_check_successful = true;
    } while(false);

    return 0;
}

static void thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    UpdateChecker* instance = context;

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
        instance->thread = NULL;

        if(instance->done_callback) {
            instance->done_callback(
                instance->was_check_successful,
                &(UpdaterCheckerInfo){
                    .url = instance->update_metadata.url,
                    .id = instance->update_metadata.id,
                    .version = instance->update_metadata.version,
                    .sha256 = instance->update_metadata.sha256,
                    .changelog = instance->update_metadata.changelog,
                },
                instance->done_callback_context);
        }

        furi_semaphore_release(instance->run_lock);

        FURI_LOG_D(TAG, "Finished checking for update");
    }
}

UpdateChecker* update_checker_alloc(void) {
    UpdateChecker* instance = malloc(sizeof(*instance));

    instance->thread = NULL;
    instance->run_lock = furi_semaphore_alloc(1, 1);

    instance->url = furi_string_alloc();
    instance->channel_id = furi_string_alloc();

    instance->update_metadata.url = furi_string_alloc();
    instance->update_metadata.id = furi_string_alloc();
    instance->update_metadata.version = furi_string_alloc();
    instance->update_metadata.sha256 = furi_string_alloc();
    instance->update_metadata.changelog = furi_string_alloc();

    instance->was_check_successful = false;

    instance->done_callback = NULL;
    instance->done_callback_context = NULL;

    return instance;
}

void update_checker_free(UpdateChecker* instance) {
    furi_assert(instance);

    furi_check(furi_semaphore_get_count(instance->run_lock) > 0);

    furi_semaphore_free(instance->run_lock);

    furi_string_free(instance->url);
    furi_string_free(instance->channel_id);

    furi_string_free(instance->update_metadata.url);
    furi_string_free(instance->update_metadata.id);
    furi_string_free(instance->update_metadata.version);
    furi_string_free(instance->update_metadata.sha256);
    furi_string_free(instance->update_metadata.changelog);

    free(instance);
}

void update_checker_set_done_callback(
    UpdateChecker* instance,
    UpdateCheckerDoneCallback callback,
    void* context) {
    furi_assert(instance);

    furi_check(furi_semaphore_get_count(instance->run_lock) > 0);

    instance->done_callback = callback;
    instance->done_callback_context = context;
}

bool update_checker_run(UpdateChecker* instance, const char* url, const char* channel_id) {
    furi_assert(instance);
    furi_assert(url);
    furi_assert(channel_id);

    FURI_LOG_D(TAG, "Beginning checking for update...");

    bool is_success;
    do {
        if(furi_semaphore_acquire(instance->run_lock, 0) != FuriStatusOk) {
            is_success = false;
            FURI_LOG_D(TAG, "Active update check is running, aborting");
            break;
        }

        furi_string_set_str(instance->url, url);
        furi_string_set_str(instance->channel_id, channel_id);

        instance->thread =
            furi_thread_alloc_ex(THREAD_NAME, THREAD_STACK_SIZE, thread_callback, instance);
        furi_thread_set_state_context(instance->thread, instance);
        furi_thread_set_state_callback(instance->thread, thread_state_callback);
        furi_thread_start(instance->thread);

        is_success = true;
    } while(false);

    return is_success;
}
