#include "settings_fw_loader.h"

#include <storage/storage.h>
#include <wifi/wifi.h>
#include <applications/system/fetch/helpers/fetch_client.h>
#include <applications/system/fetch/helpers/fetch_file_save.h>

#define TAG "SettingsFwLoader"

#define SETTINGS_FW_LOADER_FILE_PATH EXT_PATH("update/upload.tar")

struct SettingsFwLoader {
    FuriThread* thread;
    FuriSemaphore* is_processing_semaphore;
    FuriString* url;
    FetchClient* fetch_client;
    FetchFileSave* file_save;
    FuriMessageQueue* status_queue;
    FuriStreamBuffer* state_msg;
    bool error;

    SettingsFwLoaderCallbackStatus callback_status;
    void* context_status;
    SettingsFwLoaderCallbackState callback_state;
    void* context_state;
};

SettingsFwLoader* settings_fw_loader_alloc(void) {
    SettingsFwLoader* instance = malloc(sizeof(SettingsFwLoader));
    instance->url = furi_string_alloc();
    instance->is_processing_semaphore = furi_semaphore_alloc(1, 1);
    instance->status_queue = furi_message_queue_alloc(10, sizeof(FetchClientStatus));
    instance->state_msg = furi_stream_buffer_alloc(512, 1);
    instance->thread = NULL;
    return instance;
}

void settings_fw_loader_free(SettingsFwLoader* instance) {
    furi_check(instance);
    furi_check(!furi_semaphore_get_space(instance->is_processing_semaphore));
    furi_message_queue_free(instance->status_queue);
    furi_string_free(instance->url);
    furi_semaphore_free(instance->is_processing_semaphore);
    furi_stream_buffer_free(instance->state_msg);
    free(instance);
}

static bool settings_fw_loader_is_connected(SettingsFwLoader* instance) {
    furi_assert(instance);
    UNUSED(instance);

    bool ret = false;
    Wifi* wifi = furi_record_open(RECORD_WIFI);
    WifiInfo wifi_info;
    const WifiStatus status = wifi_get_info(wifi, &wifi_info);

    if(status != WifiStatusOk) {
        FURI_LOG_D(TAG, "Failed to get Wifi info: %d", status);
    } else if(wifi_info.state != WifiStateUp) {
        FURI_LOG_D(TAG, "Wifi is not connected");
    } else {
        FURI_LOG_D(TAG, "Wifi is connected");
        ret = true;
    }
    furi_record_close(RECORD_WIFI);
    return ret;
}

static int32_t settings_fw_loader_check_wifi_connected(SettingsFwLoader* instance) {
    furi_assert(instance);
    bool ret = true;
    FuriString* state_str = furi_string_alloc_printf("Wait on connecting to WiFi...");
    if(instance->callback_state) {
        instance->callback_state(state_str, instance->context_state);
    }

    if(!settings_fw_loader_is_connected(instance)) {
        instance->error = true;
        furi_string_printf(state_str, "Not connected to WiFi");
        if(instance->callback_state) {
            instance->callback_state(state_str, instance->context_state);
        }
        ret = false;
    }
    furi_string_free(state_str);
    return ret;
}

//########## Thread ##########

static void
    settings_fw_loader_callback_file_write_data(uint8_t* data, size_t data_size, void* context) {
    furi_assert(context);
    SettingsFwLoader* instance = context;
    furi_assert(instance->file_save);
    if(data_size > 0) {
        fetch_file_save_write(instance->file_save, data, data_size);
    } else {
        FURI_LOG_W(TAG, "No data received for file write");
    }
}

void settings_fw_loader_callback_status(FetchClientStatus status, void* context) {
    furi_assert(context);
    SettingsFwLoader* instance = context;
    furi_assert(instance);
    furi_message_queue_put(instance->status_queue, &status, FuriWaitForever);
}

static void settings_fw_loader_callback_state(const char* error, void* context) {
    furi_assert(context);
    SettingsFwLoader* instance = context;
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

static void settings_fw_loader_thread_state_callback(
    FuriThread* thread,
    FuriThreadState state,
    void* context) {
    furi_assert(thread);
    SettingsFwLoader* instance = context;

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
        FURI_LOG_D(TAG, "Stop");
        furi_semaphore_release(instance->is_processing_semaphore);
        instance->thread = NULL;
    }
}

static int32_t settings_fw_loader_thread_callback(void* context) {
    furi_assert(context);
    SettingsFwLoader* instance = context;
    FURI_LOG_D(TAG, "Start");

    if(!settings_fw_loader_check_wifi_connected(instance)) {
        return 0;
    }

    FuriString* path = furi_string_alloc_printf(SETTINGS_FW_LOADER_FILE_PATH);
    FuriString* url = instance->url;
    instance->fetch_client = fetch_client_alloc();
    fetch_client_set_context(instance->fetch_client, instance);
    instance->file_save = fetch_file_save_alloc(path);
    if(!instance->file_save) {
        FURI_LOG_E(TAG, "Failed to open file %s", furi_string_get_cstr(path));
        furi_crash();
        fetch_client_free(instance->fetch_client);
        furi_string_free(path);
        return 0;
    }
    fetch_client_set_callback_raw_data(
        instance->fetch_client, settings_fw_loader_callback_file_write_data);
    fetch_client_set_callback_status(instance->fetch_client, settings_fw_loader_callback_status);

    fetch_client_set_callback_error(instance->fetch_client, settings_fw_loader_callback_state);

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
                SettingsFwLoaderStatus status_converted = {
                    .total_download_size = status.total_download_size,
                    .received_download_size = status.received_download_size,
                    .speed_bytes_per_sec = status.speed_bytes_per_sec,
                };
                instance->callback_status(status_converted, instance->context_status);
            }
        }
        while(furi_stream_buffer_bytes_available(instance->state_msg)) {
            uint8_t buffer[128];
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

    if(!instance->error) {
        FURI_LOG_D(TAG, "Firmware download complete to %s", furi_string_get_cstr(path));
    } else {
        fetch_file_save_remove(instance->file_save);
        FURI_LOG_E(TAG, "Error occurred during firmware download");
    }

    // exit thread when done
    fetch_file_save_free(instance->file_save);
    instance->file_save = NULL;
    fetch_client_free(instance->fetch_client);
    instance->fetch_client = NULL;
    furi_string_free(path);
    FURI_LOG_D(TAG, "Stopping thread");

    return 0;
}

void settings_fw_loader_run(SettingsFwLoader* instance, const char* url) {
    furi_check(instance);

    if(furi_semaphore_get_space(instance->is_processing_semaphore)) {
        FURI_LOG_W(TAG, "FW Loader is already running");
        return;
    }

    furi_semaphore_acquire(instance->is_processing_semaphore, FuriWaitForever);
    furi_string_set(instance->url, url);
    instance->thread = furi_thread_alloc_ex(
        "SettingsFwLoader", 2048, settings_fw_loader_thread_callback, instance);
    furi_thread_set_state_context(instance->thread, instance);
    furi_thread_set_state_callback(instance->thread, settings_fw_loader_thread_state_callback);

    FURI_LOG_D(TAG, "Starting thread");

    furi_thread_start(instance->thread);
}

bool settings_fw_loader_is_processing_done(SettingsFwLoader* instance) {
    furi_check(instance);
    return !furi_semaphore_get_space(instance->is_processing_semaphore);
}

void settings_fw_loader_set_status_callback(
    SettingsFwLoader* instance,
    SettingsFwLoaderCallbackStatus callback,
    void* context) {
    furi_check(instance);
    instance->callback_status = callback;
    instance->context_status = context;
}

void settings_fw_loader_set_state_callback(
    SettingsFwLoader* instance,
    SettingsFwLoaderCallbackState callback,
    void* context) {
    furi_check(instance);
    instance->callback_state = callback;
    instance->context_state = context;
}
