#include "fetch_loader.h"

#include <storage/storage.h>
#include <toolbox/fetch/fetch_client.h>
#include <toolbox/fetch/fetch_file_save.h>

#define TAG "FetchLoader"

#define FETCH_LOADER_SIZE_BUFFER              128
#define FETCH_LOADER_THREAD_STACK_SIZE        2048
#define FETCH_LOADER_WAIT_FORCED_DONE_MS      5000
#define FETCH_LOADER_WAIT_FORCED_DONE_STEP_MS 100

struct FetchLoader {
    FuriThread* thread;
    FuriSemaphore* is_processing_semaphore;
    FuriString* url;
    FuriString* path;
    FetchClient* fetch_client;
    FetchFileSave* file_save;
    FuriMessageQueue* status_queue;
    FuriStreamBuffer* state_msg;
    _Atomic bool error;
    _Atomic bool stop_requested;

    FetchLoaderCallbackStatus callback_status;
    void* context_status;
    FetchLoaderCallbackState callback_state;
    void* context_state;
    FetchLoaderCallbackDone callback_done;
    void* context_done;
};

//########## Fetch Client Callbacks ##########

static void fetch_loader_callback_file_write_data(uint8_t* data, size_t data_size, void* context) {
    furi_assert(context);
    FetchLoader* instance = context;
    furi_assert(instance->file_save);
    if(data_size > 0) {
        fetch_file_save_write(instance->file_save, data, data_size);
    } else {
        FURI_LOG_W(TAG, "No data received for file write");
    }
}

void fetch_loader_callback_status(FetchClientStatus status, void* context) {
    furi_assert(context);
    FetchLoader* instance = context;
    furi_assert(instance);
    furi_message_queue_put(instance->status_queue, &status, FuriWaitForever);
}

static void fetch_loader_callback_state(const char* error, void* context) {
    furi_assert(context);
    FetchLoader* instance = context;
    furi_assert(instance);
    FuriString* state_str = furi_string_alloc_printf("Error: %s\r\n", error);
    furi_stream_buffer_send(
        instance->state_msg,
        (uint8_t*)furi_string_get_cstr(state_str),
        furi_string_size(state_str),
        FuriWaitForever);
    furi_string_free(state_str);
    instance->error = true;
}

FetchLoader* fetch_loader_alloc(void) {
    FetchLoader* instance = malloc(sizeof(FetchLoader));
    instance->url = furi_string_alloc();
    instance->path = furi_string_alloc();
    instance->is_processing_semaphore = furi_semaphore_alloc(1, 1);
    instance->status_queue = furi_message_queue_alloc(10, sizeof(FetchClientStatus));
    instance->state_msg = furi_stream_buffer_alloc(512, 1);
    instance->fetch_client = fetch_client_alloc();
    instance->thread = NULL;
    instance->error = false;
    instance->stop_requested = false;

    fetch_client_set_context(instance->fetch_client, instance);
    fetch_client_set_callback_raw_data(
        instance->fetch_client, fetch_loader_callback_file_write_data);
    fetch_client_set_callback_status(instance->fetch_client, fetch_loader_callback_status);
    fetch_client_set_callback_error(instance->fetch_client, fetch_loader_callback_state);

    return instance;
}

void fetch_loader_free(FetchLoader* instance) {
    furi_check(instance);
    furi_check(!furi_semaphore_get_space(instance->is_processing_semaphore));
    furi_check(!instance->thread);
    furi_check(!instance->file_save);

    fetch_client_free(instance->fetch_client);
    furi_message_queue_free(instance->status_queue);
    furi_string_free(instance->url);
    furi_string_free(instance->path);
    furi_semaphore_free(instance->is_processing_semaphore);
    furi_stream_buffer_free(instance->state_msg);
    free(instance);
}

void fetch_loader_forced_done(FetchLoader* instance) {
    furi_check(instance);

    if(!fetch_client_is_processing_done(instance->fetch_client)) {
        fetch_client_forced_done(instance->fetch_client);
        int timeout = FETCH_LOADER_WAIT_FORCED_DONE_MS;
        FURI_LOG_D(TAG, "Waiting for fetch client to stop...");
        instance->stop_requested = true;
        while(furi_semaphore_get_space(instance->is_processing_semaphore) &&
              (timeout -= FETCH_LOADER_WAIT_FORCED_DONE_STEP_MS) > 0) {
            furi_delay_ms(FETCH_LOADER_WAIT_FORCED_DONE_STEP_MS);
            FURI_LOG_D(TAG, "Waiting stop...");
        }
        if(timeout <= 0) {
            FURI_LOG_E(TAG, "Timeout waiting for fetch loader to stop");
            furi_crash();
        }
    }
}

//########## Thread ##########

static void
    fetch_loader_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    furi_assert(thread);
    FetchLoader* instance = context;

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
        instance->thread = NULL;
        FURI_LOG_D(TAG, "Stop");
        furi_semaphore_release(instance->is_processing_semaphore);

        if(instance->callback_done) {
            FetchLoaderDoneStatus done_status = (instance->stop_requested) ?
                                                    FetchLoaderDoneStatusAbort :
                                                (instance->error) ? FetchLoaderDoneStatusFailure :
                                                                    FetchLoaderDoneStatusSuccess;

            instance->callback_done(done_status, instance->context_done);
        }
    }
}

static int32_t fetch_loader_thread_callback(void* context) {
    furi_assert(context);
    FetchLoader* instance = context;
    FURI_LOG_D(TAG, "Start");

    if(instance->callback_state) {
        FuriString* state_str = furi_string_alloc_printf("Connecting to server...");
        instance->callback_state(state_str, instance->context_state);
        furi_string_free(state_str);
    }

    FuriString* path = instance->path;
    FuriString* url = instance->url;
    instance->file_save = fetch_file_save_alloc(path);
    if(!instance->file_save) {
        FURI_LOG_E(TAG, "Failed to open file %s", furi_string_get_cstr(path));
        return 0;
    }

    fetch_client_run(instance->fetch_client, url);

    FetchClientStatus status;
    while(!fetch_client_is_processing_done(instance->fetch_client) ||
          furi_stream_buffer_bytes_available(instance->state_msg)) {
        if(furi_message_queue_get(instance->status_queue, &status, 200) == FuriStatusOk) {
            FURI_LOG_D(
                TAG,
                "Status: %d/%d %ld",
                status.received_download_size,
                status.total_download_size,
                status.speed_bytes_per_sec);
            if(instance->callback_status) {
                FetchLoaderStatus status_converted = {
                    .total_download_size = status.total_download_size,
                    .received_download_size = status.received_download_size,
                    .speed_bytes_per_sec = status.speed_bytes_per_sec,
                };
                instance->callback_status(&status_converted, instance->context_status);
            }
        }

        while(furi_stream_buffer_bytes_available(instance->state_msg)) {
            uint8_t buffer[FETCH_LOADER_SIZE_BUFFER];
            size_t bytes_read =
                furi_stream_buffer_receive(instance->state_msg, buffer, sizeof(buffer), 0);
            if(bytes_read > 0) {
                FURI_LOG_E(TAG, "Error: %.*s", (int)bytes_read, buffer);
                if(instance->callback_state) {
                    FuriString* state_str =
                        furi_string_alloc_printf("%.*s", (int)bytes_read, buffer);
                    instance->callback_state(state_str, instance->context_state);
                    furi_string_free(state_str);
                }
            }
        }
    }

    if(!instance->error && !instance->stop_requested) {
        FURI_LOG_D(TAG, "File download complete to %s", furi_string_get_cstr(path));
    } else {
        fetch_file_save_remove(instance->file_save);
        FURI_LOG_E(TAG, "Error occurred during download");
    }

    // exit thread when done
    fetch_file_save_free(instance->file_save);
    instance->file_save = NULL;
    FURI_LOG_D(TAG, "Stopping thread");

    return 0;
}

bool fetch_loader_run(FetchLoader* instance, const char* url, const char* path) {
    furi_check(instance);

    if(furi_semaphore_get_space(instance->is_processing_semaphore)) {
        FURI_LOG_W(TAG, "Fetch loader is already running");
        return false;
    }

    furi_semaphore_acquire(instance->is_processing_semaphore, FuriWaitForever);
    furi_string_set(instance->url, url);
    furi_string_set(instance->path, path);
    instance->thread = furi_thread_alloc_ex(
        "FetchLoader", FETCH_LOADER_THREAD_STACK_SIZE, fetch_loader_thread_callback, instance);
    furi_thread_set_state_context(instance->thread, instance);
    furi_thread_set_state_callback(instance->thread, fetch_loader_thread_state_callback);

    FURI_LOG_D(TAG, "Starting thread");

    furi_thread_start(instance->thread);
    return true;
}

bool fetch_loader_is_processing_done(FetchLoader* instance) {
    furi_check(instance);
    return !furi_semaphore_get_space(instance->is_processing_semaphore);
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
