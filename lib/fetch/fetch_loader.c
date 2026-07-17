#include "fetch_loader.h"

#include <furi.h>

#include <storage_utils/temp_file.h>

#include "fetch.h"

#define TAG "FetchLoader"

#define FETCH_LOADER_THREAD_NAME       TAG
#define FETCH_LOADER_THREAD_STACK_SIZE 8192

struct FetchLoader {
    FuriString* remote_url;
    FuriString* file_path;

    Fetch* fetch;
    TempFile* file;

    FetchLoaderProgressCallback progress_callback;
    FetchLoaderStateCallback state_callback;
    FetchLoaderDoneCallback done_callback;
    void* callback_context;

    _Atomic bool is_error_occurred;
    _Atomic bool is_stop_requested;
};

static FetchStatus fetch_loader_get_status(const FetchLoader* instance) {
    FetchStatus status;

    if(instance->is_stop_requested) {
        status = FetchStatusAborted;
    } else if(instance->is_error_occurred) {
        status = FetchStatusError;
    } else {
        status = FetchStatusOk;
    }

    return status;
}

static void fetch_loader_notify_state(const FetchLoader* instance, const char* message) {
    if(instance->state_callback) {
        instance->state_callback(message, instance->callback_context);
    }
}

static void
    fetch_loader_notify_progress(const FetchLoader* instance, const FetchProgress* progress) {
    if(instance->progress_callback) {
        instance->progress_callback(progress, instance->callback_context);
    }
}

static void fetch_loader_notify_done(const FetchLoader* instance, FetchStatus status) {
    if(instance->done_callback) {
        instance->done_callback(status, instance->callback_context);
    }
}

static void fetch_file_out_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    FetchLoader* instance = context;

    if(data_size > 0) {
        if(!temp_file_write(instance->file, data, data_size)) {
            fetch_stop(instance->fetch);

            FURI_LOG_E(TAG, "Failed to write to output file");
            instance->is_error_occurred = true;
        }

    } else {
        FURI_LOG_W(TAG, "No data received for file write");
    }
}

void fetch_progress_callback(const FetchProgress* progress, void* context) {
    furi_assert(context);
    FetchLoader* instance = context;

    FURI_LOG_D(
        TAG,
        "Status: %zu/%zu %lu",
        progress->received_download_size,
        progress->total_download_size,
        progress->speed_bytes_per_sec);

    fetch_loader_notify_progress(instance, progress);
}

static void fetch_error_callback(const char* error, void* context) {
    furi_assert(context);
    FetchLoader* instance = context;

    FURI_LOG_E(TAG, "Error: %s", error);

    fetch_loader_notify_state(instance, error);
    instance->is_error_occurred = true;
}

FetchLoader* fetch_loader_alloc(void) {
    FetchLoader* instance = malloc(sizeof(FetchLoader));

    instance->remote_url = furi_string_alloc();
    instance->file_path = furi_string_alloc();
    instance->fetch = fetch_alloc();
    instance->file = temp_file_alloc(furi_record_open(RECORD_STORAGE));

    fetch_set_callback_context(instance->fetch, instance);
    fetch_set_rx_data_callback(instance->fetch, fetch_file_out_callback);
    fetch_set_progress_callback(instance->fetch, fetch_progress_callback);
    fetch_set_error_callback(instance->fetch, fetch_error_callback);

    return instance;
}

void fetch_loader_free(FetchLoader* instance) {
    furi_check(instance);

    temp_file_free(instance->file);
    fetch_free(instance->fetch);
    furi_string_free(instance->remote_url);
    furi_string_free(instance->file_path);

    free(instance);

    furi_record_close(RECORD_STORAGE);
}

void fetch_loader_stop(FetchLoader* instance) {
    furi_check(instance);

    instance->is_stop_requested = true;
    fetch_stop(instance->fetch);
}

static bool fetch_loader_prepare(FetchLoader* instance) {
    const char* file_path = furi_string_get_cstr(instance->file_path);
    const bool success = temp_file_create(instance->file, file_path);

    if(!success) {
        FURI_LOG_E(TAG, "Failed to create file %s", file_path);
        instance->is_error_occurred = true;
    }

    return success;
}

static void fetch_loader_finalize(FetchLoader* instance) {
    const char* file_path = furi_string_get_cstr(instance->file_path);

    if(!(instance->is_error_occurred || instance->is_stop_requested)) {
        FURI_LOG_D(TAG, "File download complete to %s", file_path);

    } else {
        temp_file_remove(instance->file);

        if(instance->is_error_occurred) {
            FURI_LOG_E(TAG, "Error occurred during download");
        } else {
            FURI_LOG_W(TAG, "Download aborted");
        }
    }
}

static void
    fetch_loader_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    furi_assert(thread);
    FetchLoader* instance = context;

    if(state == FuriThreadStateStopped) {
        fetch_loader_notify_done(instance, fetch_loader_get_status(instance));
        furi_thread_free(thread);
    }
}

static int32_t fetch_loader_thread_callback(void* context) {
    furi_assert(context);
    FetchLoader* instance = context;

    if(fetch_loader_prepare(instance)) {
        fetch_loader_notify_state(instance, "Connecting to server...");

        const FetchRequest request = {
            .url = furi_string_get_cstr(instance->remote_url),
        };

        fetch_run(instance->fetch, &request);

        fetch_loader_finalize(instance);
    }

    return 0;
}

static void fetch_loader_reset(FetchLoader* instance) {
    instance->is_error_occurred = false;
    instance->is_stop_requested = false;
}

void fetch_loader_start(FetchLoader* instance, const char* remote_url, const char* file_path) {
    furi_check(instance);
    furi_check(remote_url);
    furi_check(file_path);

    fetch_loader_reset(instance);

    furi_string_set(instance->remote_url, remote_url);
    furi_string_set(instance->file_path, file_path);

    FuriThread* thread = furi_thread_alloc_ex(
        FETCH_LOADER_THREAD_NAME,
        FETCH_LOADER_THREAD_STACK_SIZE,
        fetch_loader_thread_callback,
        instance);

    furi_thread_set_state_context(thread, instance);
    furi_thread_set_state_callback(thread, fetch_loader_thread_state_callback);

    furi_thread_start(thread);
}

void fetch_loader_set_callback_context(FetchLoader* instance, void* context) {
    furi_check(instance);
    instance->callback_context = context;
}

void fetch_loader_set_progress_callback(
    FetchLoader* instance,
    FetchLoaderProgressCallback callback) {
    furi_check(instance);
    instance->progress_callback = callback;
}

void fetch_loader_set_state_callback(FetchLoader* instance, FetchLoaderStateCallback callback) {
    furi_check(instance);
    instance->state_callback = callback;
}

void fetch_loader_set_done_callback(FetchLoader* instance, FetchLoaderDoneCallback callback) {
    furi_check(instance);
    instance->done_callback = callback;
}
