#include <toolbox/fetch/fetch_client.h>
#include <toolbox/fetch/fetch_file_save.h>
#include <toolbox/argparse.h>

#include <furi.h>
#include <cli/cli_ansi.h>
#include <cli/cli_command.h>

#define TAG "Fetch"

typedef struct {
    const char* url;
    const char* output_path;
    const char* request_method;
    const char* request_body;
    bool is_full_output;
} FetchParams;

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

static void fetch_client_callback_status(FetchClientStatus status, void* context) {
    furi_assert(context);
    Fetch* instance = context;
    furi_assert(instance);
    furi_message_queue_put(instance->status_queue, &status, FuriWaitForever);
}

static Fetch* fetch_alloc() {
    Fetch* instance = malloc(sizeof(Fetch));

    instance->error = false;
    instance->buffer_rx = furi_stream_buffer_alloc(1024 * 4, 1);
    instance->status_queue = furi_message_queue_alloc(10, sizeof(FetchClientStatus));
    instance->fetch_client = fetch_client_alloc();
    instance->file_save = fetch_file_save_alloc();
    fetch_client_set_context(instance->fetch_client, instance);

    return instance;
}

static void fetch_free(Fetch* instance) {
    fetch_file_save_free(instance->file_save);
    fetch_client_free(instance->fetch_client);
    furi_stream_buffer_free(instance->buffer_rx);
    furi_message_queue_free(instance->status_queue);

    free(instance);
}

static bool fetch_url(PipeSide* pipe, const FetchParams* params) {
    UNUSED(pipe);

    bool ret = false;

    Fetch* instance = fetch_alloc();

    const char* output_path = params->output_path;

    if(output_path != NULL) {
        if(!fetch_file_save_open(instance->file_save, FetchFileSaveFlagNone, output_path)) {
            printf(ANSI_FG_RED "Error: Failed to open file %s\r\n" ANSI_RESET, output_path);
            fetch_free(instance);
            return ret;
        }

        fetch_client_set_callback_raw_data(
            instance->fetch_client, fetch_client_callback_file_write_data);

        fetch_client_set_callback_status(instance->fetch_client, fetch_client_callback_status);

    } else {
        fetch_client_set_callback_raw_data(instance->fetch_client, fetch_client_callback_raw_data);
    }

    if(params->is_full_output) {
        fetch_client_set_callback_header(instance->fetch_client, fetch_client_callback_header);
    }

    fetch_client_set_callback_error(instance->fetch_client, fetch_client_callback_error);

    fetch_client_run(instance->fetch_client, params->url);

    const char spin_chars[] = "|/-\\";
    uint8_t spin_chars_index = 0;
    bool flag_waiting_receive_data = true;
    FetchClientStatus status;

    while(!fetch_client_is_processing_done(instance->fetch_client) ||
          furi_stream_buffer_bytes_available(instance->buffer_rx)) {
        if(furi_message_queue_get(instance->status_queue, &status, 200) == FuriStatusOk) {
            flag_waiting_receive_data = false;
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

        if(!furi_stream_buffer_bytes_available(instance->buffer_rx) && flag_waiting_receive_data) {
            printf("\rWaiting... %c", spin_chars[spin_chars_index]);
            fflush(stdout);
            spin_chars_index = (spin_chars_index + 1) % 4;
            continue;
        }

        if(flag_waiting_receive_data) {
            printf("\r                      \r");
            flag_waiting_receive_data = false;
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
    if(params->output_path) {
        if(!instance->error) {
            printf(ANSI_FG_GREEN "File successfully saved to %s\r\n" ANSI_RESET, output_path);
            ret = true;
        } else {
            fetch_file_save_remove(instance->file_save);
            printf(ANSI_FG_RED "Error: Failed to save file to %s\r\n" ANSI_RESET, output_path);
        }
    }

    fetch_free(instance);

    return ret;
}

static void fetch_command_print_usage(void) {
    printf("Usage:\r\n"
           "\tfetch [options] <url>\r\n"
           "Options:\r\n"
           "\t-o Output file path\r\n"
           "\t-d HTTP POST/PUT data\r\n"
           "\t-H Custom header(s)\r\n"
           "\t-X Request method\r\n"
           "\t-v Enable full output\r\n");
}

static void fetch_option_callback(char opt, const char* optarg, void* context) {
    furi_assert(context);
    FetchParams* params = context;

    if(opt == '\0') {
        params->url = optarg;
    } else if(opt == 'o') {
        params->output_path = optarg;
    } else if(opt == 'd') {
        params->request_body = optarg;
    } else if(opt == 'H') {
        // TODO: Headers
    } else if(opt == 'X') {
        params->request_body = optarg;
    } else if(opt == 'v') {
        params->is_full_output = true;
    }
}

static bool fetch_validate_params(const FetchParams* params) {
    bool is_valid = false;

    do {
        if(params->url == NULL) {
            printf("Error: no url specified\r\n\n");
            break;
        }

        is_valid = true;

    } while(false);

    return is_valid;
}

void fetch_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);

    FetchParams params = {0};

    if(parse_args(args, "o:d:H:X:v", fetch_option_callback, &params) &&
       fetch_validate_params(&params)) {
        fetch_url(pipe, &params);
    } else {
        fetch_command_print_usage();
    }
}
