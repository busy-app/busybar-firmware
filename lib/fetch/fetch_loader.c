#include "fetch_loader.h"

#include "fetch.h"
#include "fetch_file_save.h"

#define TAG "FetchLoader"

#define FETCH_LOADER_SIZE_BUFFER              128
#define FETCH_LOADER_THREAD_STACK_SIZE        8192
#define FETCH_LOADER_WAIT_FORCED_DONE_MS      5000
#define FETCH_LOADER_WAIT_FORCED_DONE_STEP_MS 100

struct FetchLoader {
    FuriString* url;
    FuriString* path;
    Fetch* fetch;
    FetchFileSave* file_save;

    FetchLoaderCallbackStatus callback_status;
    void* context_status;
    FetchLoaderCallbackState callback_state;
    void* context_state;
    FetchLoaderCallbackDone callback_done;
    void* context_done;

    _Atomic bool is_error;
    _Atomic bool is_stop_requested;
};

static void fetch_file_out_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    FetchLoader* instance = context;

    FetchFileSave* file_save = instance->file_save;
    furi_assert(file_save);

    if(data_size > 0) {
        fetch_file_save_write(file_save, data, data_size);
    } else {
        FURI_LOG_W(TAG, "No data received for file write");
    }
}

void fetch_progress_callback(const FetchStatus* status, void* context) {
    furi_assert(context);
    FetchLoader* instance = context;

    FURI_LOG_D(
        TAG,
        "Status: %zu/%zu %lu",
        status->received_download_size,
        status->total_download_size,
        status->speed_bytes_per_sec);

    if(instance->callback_status) {
        instance->callback_status(status, instance->context_status);
    }
}

static void fetch_error_callback(const char* error, void* context) {
    furi_assert(context);
    FetchLoader* instance = context;

    FURI_LOG_E(TAG, "Error: %s", error);

    if(instance->callback_state) {
        instance->callback_state(error, instance->context_state);
    }

    instance->is_error = true;
}

FetchLoader* fetch_loader_alloc(void) {
    FetchLoader* instance = malloc(sizeof(FetchLoader));
    instance->url = furi_string_alloc();
    instance->path = furi_string_alloc();
    instance->fetch = fetch_alloc();
    instance->file_save = fetch_file_save_alloc();

    fetch_set_callback_context(instance->fetch, instance);
    fetch_set_callback_raw_data(instance->fetch, fetch_file_out_callback);
    fetch_set_callback_status(instance->fetch, fetch_progress_callback);
    fetch_set_callback_error(instance->fetch, fetch_error_callback);

    return instance;
}

void fetch_loader_free(FetchLoader* instance) {
    furi_check(instance);

    fetch_file_save_free(instance->file_save);
    fetch_free(instance->fetch);
    furi_string_free(instance->url);
    furi_string_free(instance->path);

    free(instance);
}

void fetch_loader_stop(FetchLoader* instance) {
    furi_check(instance);

    instance->is_stop_requested = true;
    fetch_stop(instance->fetch);
}

static void
    fetch_loader_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    furi_assert(thread);
    FetchLoader* instance = context;

    if(state == FuriThreadStateStopped) {
        FURI_LOG_D(TAG, "Stop");

        if(instance->callback_done) {
            FetchLoaderDoneStatus done_status;

            if(instance->is_stop_requested) {
                done_status = FetchLoaderDoneStatusAbort;
            } else if(instance->is_error) {
                done_status = FetchLoaderDoneStatusFailure;
            } else {
                done_status = FetchLoaderDoneStatusSuccess;
            }

            instance->callback_done(done_status, instance->context_done);
        }

        furi_thread_free(thread);
    }
}

static int32_t fetch_loader_thread_callback(void* context) {
    furi_assert(context);
    FetchLoader* instance = context;
    FURI_LOG_D(TAG, "Start");

    if(instance->callback_state) {
        instance->callback_state("Connecting to server...", instance->context_state);
    }

    const char* path = furi_string_get_cstr(instance->path);

    if(!fetch_file_save_open(instance->file_save, path)) {
        FURI_LOG_E(TAG, "Failed to open file %s", path);
        return 0;
    }

    const FetchRequest request = {
        .url = furi_string_get_cstr(instance->url),
    };

    fetch_exec(instance->fetch, &request);

    if(!instance->is_error && !instance->is_stop_requested) {
        FURI_LOG_D(TAG, "File download complete to %s", path);
    } else {
        fetch_file_save_remove(instance->file_save);
        FURI_LOG_E(TAG, "Error occurred during download");
    }

    FURI_LOG_D(TAG, "Stopping thread");

    return 0;
}

void fetch_loader_start(FetchLoader* instance, const char* url, const char* path) {
    furi_check(instance);

    furi_string_set(instance->url, url);
    furi_string_set(instance->path, path);

    FuriThread* thread = furi_thread_alloc_ex(
        "FetchLoader", FETCH_LOADER_THREAD_STACK_SIZE, fetch_loader_thread_callback, instance);
    furi_thread_set_state_context(thread, instance);
    furi_thread_set_state_callback(thread, fetch_loader_thread_state_callback);

    FURI_LOG_D(TAG, "Starting thread");

    furi_thread_start(thread);
}

void fetch_loader_set_status_callback(
    FetchLoader* instance,
    FetchLoaderCallbackStatus callback,
    void* context) {
    furi_check(instance);
    instance->callback_status = callback;
    instance->context_status = context;
}

void fetch_loader_set_state_callback(
    FetchLoader* instance,
    FetchLoaderCallbackState callback,
    void* context) {
    furi_check(instance);
    instance->callback_state = callback;
    instance->context_state = context;
}

void fetch_loader_set_done_callback(
    FetchLoader* instance,
    FetchLoaderCallbackDone callback,
    void* context) {
    furi_check(instance);

    instance->callback_done = callback;
    instance->context_done = context;
}
