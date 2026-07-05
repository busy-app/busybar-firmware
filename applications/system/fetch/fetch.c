#include <toolbox/fetch/fetch_client.h>
#include <toolbox/fetch/fetch_file_save.h>
#include <toolbox/argparse.h>

#include <furi.h>
#include <cli/cli_ansi.h>
#include <cli/cli_command.h>

#define TAG "Fetch"

#define PROGRESS_BAR_SEGMENT_COUNT (20)

typedef struct {
    const char* url;
    const char* output_path;
    const char* request_method;
    const char* request_body;
    bool is_full_output;
} FetchParams;

typedef struct {
    FuriEventLoop* event_loop;
    FuriStreamBuffer* buffer_rx;
    FetchClient* fetch_client;
    FuriMessageQueue* status_queue;
    FetchFileSave* file_save;
    bool is_error;
} Fetch;

static void fetch_client_callback_raw_data(uint8_t* data, size_t data_size, void* context) {
    furi_assert(context);
    Fetch* instance = context;
    furi_stream_buffer_send(instance->buffer_rx, data, data_size, FuriWaitForever);
}

static void fetch_client_callback_file_write_data(uint8_t* data, size_t data_size, void* context) {
    furi_assert(context);
    Fetch* instance = context;

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

    FuriString* error_str =
        furi_string_alloc_printf(ANSI_FG_RED "Error: " ANSI_RESET "%s\r\n", error);

    furi_stream_buffer_send(
        instance->buffer_rx,
        (uint8_t*)furi_string_get_cstr(error_str),
        furi_string_size(error_str),
        FuriWaitForever);

    furi_string_free(error_str);
    instance->is_error = true;
}

static void fetch_client_callback_status(FetchClientStatus status, void* context) {
    furi_assert(context);
    Fetch* instance = context;

    furi_message_queue_put(instance->status_queue, &status, FuriWaitForever);
}

static void fetch_client_callback_done(void* context) {
    furi_assert(context);
    Fetch* instance = context;

    furi_event_loop_stop(instance->event_loop);
}

static void fetch_print_download_progress(const FetchClientStatus* status) {
    const char* units_str;
    size_t multiplier;

    if(status->total_download_size > 2048) {
        multiplier = 1024;
        units_str = "KiB";

    } else {
        multiplier = 1;
        units_str = "B";
    }

    const size_t download_size = status->received_download_size / multiplier;
    const size_t total_size = status->total_download_size / multiplier;

    const size_t download_percent = (download_size * 100) / total_size;

    printf(ANSI_BG_GREEN ANSI_FG_BLACK "\rDownloaded: [%3zu%%]" ANSI_RESET " [", download_percent);

    const size_t num_segments = (download_size * PROGRESS_BAR_SEGMENT_COUNT) / total_size;

    for(size_t i = 0; i < num_segments; i++) {
        putchar('=');
    }

    for(size_t i = num_segments; i < PROGRESS_BAR_SEGMENT_COUNT; i++) {
        putchar(' ');
    }

    const float download_speed = status->speed_bytes_per_sec / 1024.0f;

    printf(
        "] %8.2f KiB/s, %zu%s/%zu%s        ",
        download_speed,
        download_size,
        units_str,
        total_size,
        units_str);
}

static void fetch_print_download_progress_simple(const FetchClientStatus* status) {
    const float download_speed = status->speed_bytes_per_sec / 1024.0f;
    const size_t download_size = status->received_download_size / 1024;

    printf(
        ANSI_BG_GREEN ANSI_FG_BLACK "\rDownloaded: " ANSI_RESET
                                    "%8.2fKiB/s, Total: %zuKiB        ",
        download_speed,
        download_size);
}

static void fetch_status_queue_callback(FuriEventLoopObject* obj, void* context) {
    furi_assert(context);

    Fetch* instance = context;
    furi_assert(obj == instance->status_queue);

    FetchClientStatus status;

    while(furi_message_queue_get(instance->status_queue, &status, 0) == FuriStatusOk) {
        if(status.total_download_size != 0) {
            fetch_print_download_progress(&status);
        } else {
            fetch_print_download_progress_simple(&status);
        }

        fflush(stdout);
    }
}

static void fetch_stream_buffer_rx_callback(FuriEventLoopObject* obj, void* context) {
    furi_assert(context);

    Fetch* instance = context;
    furi_assert(obj == instance->buffer_rx);

    size_t bytes_read;
    uint8_t buffer[256];

    do {
        bytes_read = furi_stream_buffer_receive(instance->buffer_rx, buffer, sizeof(buffer), 0);

        for(size_t i = 0; i < bytes_read; i++) {
            const char c = buffer[i];

            if(!iscntrl(c) || isspace(c)) {
                printf("%c", c);
            } else {
                printf("\\x%02x", c);
            }

            fflush(stdout);
        }

    } while(bytes_read != 0);
}

static Fetch* fetch_alloc() {
    Fetch* instance = malloc(sizeof(Fetch));

    instance->event_loop = furi_event_loop_alloc();
    instance->buffer_rx = furi_stream_buffer_alloc(1024 * 4, 1);
    instance->status_queue = furi_message_queue_alloc(10, sizeof(FetchClientStatus));
    instance->fetch_client = fetch_client_alloc();
    instance->file_save = fetch_file_save_alloc();
    instance->is_error = false;

    fetch_client_set_context(instance->fetch_client, instance);
    fetch_client_set_callback_error(instance->fetch_client, fetch_client_callback_error);
    fetch_client_set_callback_done(instance->fetch_client, fetch_client_callback_done);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->status_queue,
        FuriEventLoopEventIn,
        fetch_status_queue_callback,
        instance);

    furi_event_loop_subscribe_stream_buffer(
        instance->event_loop,
        instance->buffer_rx,
        FuriEventLoopEventIn,
        fetch_stream_buffer_rx_callback,
        instance);

    return instance;
}

static void fetch_free(Fetch* instance) {
    furi_event_loop_unsubscribe(instance->event_loop, instance->status_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->buffer_rx);

    fetch_file_save_free(instance->file_save);
    fetch_client_free(instance->fetch_client);
    furi_stream_buffer_free(instance->buffer_rx);
    furi_message_queue_free(instance->status_queue);
    furi_event_loop_free(instance->event_loop);

    free(instance);
}

static void fetch_run(const FetchParams* params) {
    Fetch* instance = fetch_alloc();

    const char* output_path = params->output_path;

    if(output_path != NULL) {
        if(!fetch_file_save_open(instance->file_save, FetchFileSaveFlagNone, output_path)) {
            printf(ANSI_FG_RED "Error: Failed to open file %s\r\n" ANSI_RESET, output_path);
            fetch_free(instance);
            return;
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

    fetch_client_run(instance->fetch_client, params->url);
    furi_event_loop_run(instance->event_loop);

    if(output_path != NULL) {
        if(!instance->is_error) {
            printf(ANSI_FG_GREEN "File successfully saved to %s\r\n" ANSI_RESET, output_path);
        } else {
            fetch_file_save_remove(instance->file_save);
            printf(ANSI_FG_RED "Error: Failed to save file to %s\r\n" ANSI_RESET, output_path);
        }
    }

    fetch_free(instance);
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
    UNUSED(pipe);
    UNUSED(context);

    bool success = false;

    do {
        FetchParams params = {0};

        if(!parse_args(args, "o:d:H:X:v", fetch_option_callback, &params)) {
            // TODO: Error message
            break;
        }
        if(!fetch_validate_params(&params)) {
            // TODO: Error message
            break;
        }

        fetch_run(&params);
        success = true;

    } while(false);

    if(!success) {
        fetch_command_print_usage();
    }
}
