#include "fetch.h"
#include "helpers/fetch_client.h"
#include "helpers/fetch_file_save.h"

#include <furi.h>
#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <cli/cli_status.h>

#include <mongoose.h>
#include <mongoose_glue.h>
#include <wifi/wifi.h>
#include <network/network.h>

#define TAG "Fetch"

typedef struct {
    FuriStreamBuffer* buffer_rx;
    FetchClient* fetch_client;
    FuriMessageQueue* status_queue;
    FetchFileSave* file_save;
    bool error;
} Fetch;

static void fetch_client_callback_raw_data(uint8_t* data, size_t data_size, void* context) {
    furi_assert(context);
    Fetch* instance = context;
    furi_assert(instance);
    furi_stream_buffer_send(instance->buffer_rx, data, data_size, FuriWaitForever);
}

static void fetch_client_callback_file_write_data(uint8_t* data, size_t data_size, void* context) {
    furi_assert(context);
    Fetch* instance = context;
    furi_assert(instance->file_save);
    if(data_size > 0) {
        fetch_file_save_write(instance->file_save, data, data_size);
    } else {
        FURI_LOG_W(TAG, "No data received for file write");
    }
}

static void fetch_client_callback_header(uint8_t* data, size_t data_size, void* context) {
    furi_assert(context);
    Fetch* instance = context;
    furi_stream_buffer_send(instance->buffer_rx, data, data_size, FuriWaitForever);
}

static void fetch_client_callback_error(const char* error, void* context) {
    furi_assert(context);
    Fetch* instance = context;
    furi_assert(instance);
    FuriString* error_str =
        furi_string_alloc_printf(ANSI_FG_RED "Error: " ANSI_RESET "%s\r\n", error);
    furi_stream_buffer_send(
        instance->buffer_rx,
        (uint8_t*)furi_string_get_cstr(error_str),
        furi_string_size(error_str),
        FuriWaitForever);
    furi_string_free(error_str);
    instance->error = true;
}

void fetch_client_callback_status(FetchClientStatus status, void* context) {
    furi_assert(context);
    Fetch* instance = context;
    furi_assert(instance);
    furi_message_queue_put(instance->status_queue, &status, FuriWaitForever);
}

void fetch_url(PipeSide* pipe, FuriString* url, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);

    size_t pos = furi_string_search_str(url, "://", 0);
    if(pos == FURI_STRING_FAILURE) {
        FuriString* url_temp = furi_string_alloc_printf("http://%s", furi_string_get_cstr(url));
        furi_string_set(url, url_temp);
        furi_string_free(url_temp);
    }

    FuriString* path = furi_string_alloc();
    args_read_string_and_trim(args, path);

    Fetch* instance = malloc(sizeof(Fetch));
    instance->buffer_rx = furi_stream_buffer_alloc(1024 * 4, 1);

    instance->status_queue = furi_message_queue_alloc(10, sizeof(FetchClientStatus));

    instance->fetch_client = fetch_client_alloc();
    fetch_client_set_context(instance->fetch_client, instance);

    if(furi_string_size(path)) {
        instance->file_save = fetch_file_save_alloc(path);
        if(!instance->file_save) {
            printf(
                ANSI_FG_RED "Error: Failed to open file %s\r\n" ANSI_RESET,
                furi_string_get_cstr(path));
            fetch_client_free(instance->fetch_client);
            furi_stream_buffer_free(instance->buffer_rx);
            furi_message_queue_free(instance->status_queue);
            furi_string_free(path);
            free(instance);
            return;
        }

        fetch_client_set_callback_raw_data(
            instance->fetch_client, fetch_client_callback_file_write_data);

        fetch_client_set_callback_status(instance->fetch_client, fetch_client_callback_status);

    } else {
        fetch_client_set_callback_raw_data(instance->fetch_client, fetch_client_callback_raw_data);
        fetch_client_set_callback_header(instance->fetch_client, fetch_client_callback_header);
    }

    fetch_client_set_callback_error(instance->fetch_client, fetch_client_callback_error);

    fetch_client_run(instance->fetch_client, url);

    const char spin_chars[] = "|/-\\";
    uint8_t spin_chars_index = 0;
    bool flag_waiting_recive_data = true;
    FetchClientStatus status;

    while(!fetch_client_is_processing_done(instance->fetch_client) ||
          furi_stream_buffer_bytes_available(instance->buffer_rx)) {
        if(furi_message_queue_get(instance->status_queue, &status, 200) == FuriStatusOk) {
            flag_waiting_recive_data = false;
            if(status.total_download_size) {
                char* dimension = "B";
                if(status.total_download_size > 2048) {
                    status.received_download_size /= 1024;
                    status.total_download_size /= 1024;
                    dimension = "kB";
                }
                printf(
                    ANSI_BG_GREEN ANSI_FG_BLACK "\rDownloaded: [%3d%%]" ANSI_RESET " [",
                    (int)((status.received_download_size * 100) / status.total_download_size));
                size_t bars = (status.received_download_size * 20) / status.total_download_size;
                for(size_t i = 0; i < bars; i++) {
                    printf("=");
                }
                for(size_t i = bars; i < 20; i++) {
                    printf(" ");
                }
                printf(
                    "] %8.2f kB/s, %zu%s/%zu%s        ",
                    (float)status.speed_bytes_per_sec / 1024.0f,
                    status.received_download_size,
                    dimension,
                    status.total_download_size,
                    dimension);
                fflush(stdout);
            } else {
                printf(
                    ANSI_BG_GREEN ANSI_FG_BLACK "\rDownloaded: " ANSI_RESET
                                                "%8.2fkB/s, Total: %zukB        ",
                    (float)status.speed_bytes_per_sec / 1024.0f,
                    status.received_download_size / 1024);
                fflush(stdout);
            }
            continue;
        }

        if(!furi_stream_buffer_bytes_available(instance->buffer_rx) && flag_waiting_recive_data) {
            printf("\rWaiting... %c", spin_chars[spin_chars_index]);
            fflush(stdout);
            spin_chars_index = (spin_chars_index + 1) % 4;
            continue;
        }

        if(flag_waiting_recive_data) {
            printf("\r                      \r");
            flag_waiting_recive_data = false;
        }

        while(furi_stream_buffer_bytes_available(instance->buffer_rx)) {
            uint8_t buffer[256];
            size_t bytes_read =
                furi_stream_buffer_receive(instance->buffer_rx, buffer, sizeof(buffer), 0);
            if(bytes_read > 0) {
                for(size_t i = 0; i < bytes_read; i++) {
                    if(buffer[i] < 0x10 && (buffer[i] != '\r') && (buffer[i] != '\n')) {
                        printf(" [%02x] ", buffer[i]);
                    } else {
                        printf("%c", buffer[i]);
                    }
                    fflush(stdout);
                }
            }
        }
    }
    printf("\r\n");

    // If saving to file, close and report
    if(instance->file_save) {
        if(!instance->error) {
            printf(
                ANSI_FG_GREEN "File successfully saved to %s\r\n" ANSI_RESET,
                furi_string_get_cstr(path));
        } else {
            fetch_file_save_remove(instance->file_save);
            printf(
                ANSI_FG_RED "Error: Failed to save file to %s\r\n" ANSI_RESET,
                furi_string_get_cstr(path));
        }
        fetch_file_save_free(instance->file_save);
        instance->file_save = NULL;
    }

    fetch_client_free(instance->fetch_client);
    furi_stream_buffer_free(instance->buffer_rx);
    furi_message_queue_free(instance->status_queue);
    furi_string_free(path);
}

static void fetch_command_print_usage(void) {
    printf("Usage:\r\n");
    printf(
        "\tfetch <url> [path]\t : url - http(s)://example.com[:port], path - local file path to save response\r\n");
}

void fetch_command(PipeSide* pipe, FuriString* args, void* context) {
    FuriString* url = furi_string_alloc();
    do {
        if(!args_read_string_and_trim(args, url)) {
            fetch_command_print_usage();
            break;
        } else {
            fetch_url(pipe, url, args, context);
        }
    } while(false);
    furi_string_free(url);
}
