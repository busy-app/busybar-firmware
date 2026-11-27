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

    FetchRequest request;
    FetchHandle* handle;
    TempFile* file;

    FetchLoaderProgressCallback progress_callback;
    FetchLoaderStateCallback state_callback;
    FetchLoaderDoneCallback done_callback;
    void* callback_context;

    _Atomic bool is_error_occurred;
    _Atomic bool is_stop_requested;
};

static void fetch_loader_finalize(FetchLoader* instance);

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
            fetch_stop(instance->handle);

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

static void fetch_done_callback(FetchHandle* handle, void* context) {
    FetchLoader* instance = context;

    FetchStatus status = fetch_join(handle, FuriWaitForever);
    instance->handle = NULL;
    fetch_loader_finalize(instance);
    if(instance->is_stop_requested) {
        status = FetchStatusAborted;
    } else if(instance->is_error_occurred) {
        status = FetchStatusError;
    }
    fetch_loader_notify_done(instance, status);
}

FetchLoader* fetch_loader_alloc(void) {
    FetchLoader* instance = malloc(sizeof(FetchLoader));

    instance->remote_url = furi_string_alloc();
    instance->file_path = furi_string_alloc();
    instance->handle = NULL;
    instance->file = temp_file_alloc(furi_record_open(RECORD_STORAGE));

    return instance;
}

void fetch_loader_free(FetchLoader* instance) {
    furi_check(instance);

    temp_file_free(instance->file);
    if(instance->handle) {
        fetch_join(instance->handle, FuriWaitForever);
    }
    furi_string_free(instance->remote_url);
    furi_string_free(instance->file_path);

    free(instance);

    furi_record_close(RECORD_STORAGE);
}

void fetch_loader_stop(FetchLoader* instance) {
    furi_check(instance);

    instance->is_stop_requested = true;
    if(instance->handle) {
        fetch_stop(instance->handle);
    }
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
        if(!temp_file_close(instance->file)) {
            instance->is_error_occurred = true;
        }

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

    if(fetch_loader_prepare(instance)) {
        fetch_loader_notify_state(instance, "Connecting to server...");

        instance->request = (FetchRequest){
            .url = furi_string_get_cstr(instance->remote_url),
        };
        const FetchCallbacks callbacks = {
            .callback_context = instance,
            .rx_data_callback = fetch_file_out_callback,
            .progress_callback = fetch_progress_callback,
            .error_callback = fetch_error_callback,
            .done_callback = fetch_done_callback,
        };
        Fetch* fetch = furi_record_open(RECORD_FETCH);
        instance->handle = fetch_init(fetch, &instance->request, &callbacks);
        furi_record_close(RECORD_FETCH);
        if(instance->handle == NULL) {
            instance->is_error_occurred = true;
            fetch_loader_notify_state(instance, "No fetch slots available");
            fetch_loader_finalize(instance);
            fetch_loader_notify_done(instance, FetchStatusError);
        } else {
            fetch_start(instance->handle);
        }
    } else {
        fetch_loader_notify_state(instance, "Failed to create file");
        fetch_loader_notify_done(instance, FetchStatusError);
    }
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
