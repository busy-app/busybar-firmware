#include <furi.h>

#include <cli/cli_ansi.h>
#include <cli/cli_command.h>

#include <storage/storage.h>
#include <storage_utils/temp_file.h>

#include <fetch/fetch.h>

#include <toolbox/argparse.h>

#define TAG "FetchCli"

#define PROGRESS_BAR_SEGMENT_COUNT (20)

typedef enum {
    FetchCliCustomEventFinished = 1UL << 0,
} FetchCliCustomEvent;

typedef struct {
    FetchRequest request;
    const char* output_path;
    bool is_full_output;
} FetchCliParams;

typedef struct {
    Fetch* fetch;
    TempFile* output_file;
    int32_t progress_len;
    bool is_error;
} FetchCli;

static void fetch_cli_console_out(const char* data, size_t data_size) {
    for(size_t i = 0; i < data_size; i++) {
        const char c = data[i];

        if(isprint(c) || isspace(c)) {
            putchar(c);

            if(c == '\n') {
                putchar('\r');
            }

        } else {
            printf("\\x%02x", c);
        }
    }

    fflush(stdout);
}

static void fetch_console_out_callback(const void* data, size_t data_size, void* context) {
    UNUSED(context);
    fetch_cli_console_out(data, data_size);
}

static void fetch_file_out_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    FetchCli* instance = context;

    if(data_size > 0) {
        if(!temp_file_write(instance->output_file, data, data_size)) {
            fetch_stop(instance->fetch);
            instance->is_error = true;
        }

    } else {
        FURI_LOG_W(TAG, "No data received for file write");
    }
}

static void fetch_headers_callback(const void* data, size_t data_size, void* context) {
    UNUSED(context);
    fetch_cli_console_out(data, data_size);
}

static void fetch_callback_error(const char* error, void* context) {
    furi_assert(context);
    FetchCli* instance = context;

    printf("Error: %s\r\n", error);

    instance->is_error = true;
}

static int32_t fetch_cli_print_download_progress(const FetchProgress* progress) {
    int32_t print_len = 0;

    const char* units_str;
    size_t multiplier;

    if(progress->total_download_size > 2048) {
        multiplier = 1024;
        units_str = "KiB";

    } else {
        multiplier = 1;
        units_str = "B";
    }

    const size_t download_size = progress->received_download_size / multiplier;
    const size_t total_size = progress->total_download_size / multiplier;

    const size_t download_percent = (download_size * 100) / total_size;

    print_len += printf("\rDownloaded: %3zu%% [", download_percent);

    const size_t num_segments = (download_size * PROGRESS_BAR_SEGMENT_COUNT) / total_size;

    for(size_t i = 0; i < num_segments; i++) {
        putchar('=');
    }

    for(size_t i = num_segments; i < PROGRESS_BAR_SEGMENT_COUNT; i++) {
        putchar(' ');
    }

    print_len += num_segments;

    const float download_speed = progress->speed_bytes_per_sec / 1024.0f;

    print_len += printf(
        "] %8.2f KiB/s, %zu %s/%zu %s",
        download_speed,
        download_size,
        units_str,
        total_size,
        units_str);

    return print_len;
}

static int32_t fetch_cli_print_download_progress_simple(const FetchProgress* progress) {
    const float download_speed = progress->speed_bytes_per_sec / 1024.0f;
    const size_t download_size = progress->received_download_size / 1024;

    return printf("\rDownloaded: %8.2fKiB/s, %zuKiB/?KiB", download_speed, download_size);
}

static void fetch_cli_print_download_progress_space(FetchCli* instance, int32_t print_len) {
    if(print_len < 0) {
        return;
    }

    const int32_t delta = instance->progress_len - print_len;

    if(delta > 0) {
        for(int32_t i = 0; i < delta; ++i) {
            putchar(' ');
        }
    }

    instance->progress_len = print_len;
}

static void fetch_progress_callback(const FetchProgress* progress, void* context) {
    furi_assert(context);
    FetchCli* instance = context;

    int32_t print_len;

    if(progress->total_download_size != 0) {
        print_len = fetch_cli_print_download_progress(progress);
    } else {
        print_len = fetch_cli_print_download_progress_simple(progress);
    }

    fetch_cli_print_download_progress_space(instance, print_len);

    fflush(stdout);
}

static FetchCli* fetch_cli_alloc() {
    FetchCli* instance = malloc(sizeof(FetchCli));

    instance->fetch = fetch_alloc();
    instance->output_file = temp_file_alloc(furi_record_open(RECORD_STORAGE));
    instance->is_error = false;

    fetch_set_callback_context(instance->fetch, instance);
    fetch_set_error_callback(instance->fetch, fetch_callback_error);

    return instance;
}

static void fetch_cli_free(FetchCli* instance) {
    temp_file_free(instance->output_file);
    fetch_free(instance->fetch);

    free(instance);

    furi_record_close(RECORD_STORAGE);
}

static bool fetch_cli_prepare_file_output(FetchCli* instance, const char* file_path) {
    const bool success = temp_file_create(instance->output_file, file_path);

    if(success) {
        fetch_set_rx_data_callback(instance->fetch, fetch_file_out_callback);
        fetch_set_progress_callback(instance->fetch, fetch_progress_callback);

    } else {
        printf("Error: Failed to open file for writing: %s\r\n", file_path);
    }

    return success;
}

static void fetch_cli_prepare_standard_output(FetchCli* instance) {
    fetch_set_rx_data_callback(instance->fetch, fetch_console_out_callback);
}

static void fetch_cli_finalize_file_ouput(FetchCli* instance) {
    if(instance->is_error) {
        temp_file_remove(instance->output_file);
    }
}

static bool fetch_cli_prepare(FetchCli* instance, const FetchCliParams* params) {
    bool success = true;

    if(params->is_full_output) {
        fetch_set_header_callback(instance->fetch, fetch_headers_callback);
    }

    if(params->output_path != NULL) {
        success = fetch_cli_prepare_file_output(instance, params->output_path);
    } else {
        fetch_cli_prepare_standard_output(instance);
    }

    return success;
}

static void fetch_cli_finalize(FetchCli* instance, const FetchCliParams* params) {
    if(params->output_path != NULL) {
        fetch_cli_finalize_file_ouput(instance);
    }
}

static void fetch_cli_run(const FetchCliParams* params) {
    FetchCli* instance = fetch_cli_alloc();

    if(fetch_cli_prepare(instance, params)) {
        fetch_run(instance->fetch, &params->request);
    }

    fetch_cli_finalize(instance, params);
    fetch_cli_free(instance);
}

static void fetch_cli_print_usage(void) {
    printf("Usage:\r\n"
           "\tfetch [options] <url>\r\n"
           "Options:\r\n"
           "\t-o Output file path\r\n"
           "\t-d HTTP POST/PUT data\r\n"
           "\t-H Custom header(s)\r\n"
           "\t-X Request method\r\n"
           "\t-v Enable full output\r\n");
}

static void fetch_cli_option_callback(char opt, const char* optarg, void* context) {
    furi_assert(context);
    FetchCliParams* params = context;

    FetchRequest* request = &params->request;

    if(opt == '\0') {
        request->url = optarg;
    } else if(opt == 'H') {
        if(request->headers.count < FETCH_HEADERS_COUNT_MAX) {
            request->headers.data[request->headers.count++] = optarg;
        }
    } else if(opt == 'X') {
        request->method = optarg;
    } else if(opt == 'd') {
        request->body.data = optarg;
        request->body.length = strlen(optarg);
    } else if(opt == 'o') {
        params->output_path = optarg;
    } else if(opt == 'v') {
        params->is_full_output = true;
    }
}

static bool fetch_cli_validate_params(const FetchCliParams* params) {
    bool is_valid = false;

    do {
        if(params->request.url == NULL) {
            printf("Error: no url specified\r\n");
            break;
        }

        is_valid = true;

    } while(false);

    return is_valid;
}

void fetch_cli_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    bool success = false;

    do {
        FetchCliParams params = {0};

        if(!parse_args(args, "o:d:H:X:v", fetch_cli_option_callback, &params)) {
            printf("Error: invalid arguments\r\n");
            break;
        }
        if(!fetch_cli_validate_params(&params)) {
            break;
        }

        fetch_cli_run(&params);
        success = true;

    } while(false);

    if(!success) {
        fetch_cli_print_usage();
    }
}
