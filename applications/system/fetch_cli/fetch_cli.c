#include <furi.h>

#include <cli/cli_ansi.h>
#include <cli/cli_command.h>

#include <storage/storage.h>
#include <storage_utils/temp_file.h>

#include <fetch/fetch.h>

#include <toolbox/argparse.h>

#define TAG "FetchCli"

#define PROGRESS_BAR_SEGMENT_COUNT (20)

#define HTTP_PREFIX  "http://"
#define HTTPS_PREFIX "https://"

typedef enum {
    FetchCliCustomEventFinished = 1UL << 0,
} FetchCliCustomEvent;

typedef struct {
    FetchRequest request;
    FuriString* url_store;
    const char* output_path;
    bool is_full_output;
} FetchCliParams;

typedef struct {
    Fetch* fetch;
    TempFile* output_file;
    int32_t progress_len;
    bool is_error;
} FetchCli;

static void fetch_cli_print_error(const char* error_message) {
    printf("Error: %s\r\n", error_message);
}

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

static void fetch_error_callback(const char* error, void* context) {
    furi_assert(context);
    FetchCli* instance = context;

    fetch_cli_print_error(error);
    instance->is_error = true;
}

static int32_t fetch_cli_print_download_progress(const FetchProgress* progress) {
    int32_t print_len = 0;

    if(progress->total_download_size == 0) {
        return print_len;
    }

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

    if(progress->has_total_download_size) {
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
    fetch_set_error_callback(instance->fetch, fetch_error_callback);

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
        fetch_cli_print_error("Failed to open output file for writing");
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
           "\t-k (TLS) Ignore server certificate\r\n"
           "\t-a (TLS) Client auth type (\"none\" (default), \"device\" or \"cert\")\r\n"
           "\t-C (TLS) Custom client certificate file path\r\n"
           "\t-K (TLS) Custom client private key file path\r\n"
           "\t-v Enable full output\r\n");
}

static void fetch_adjust_url(FuriString* url, const char* src_url) {
    if((strncmp(src_url, HTTP_PREFIX, strlen(HTTP_PREFIX)) != 0) &&
       (strncmp(src_url, HTTPS_PREFIX, strlen(HTTPS_PREFIX)) != 0)) {
        FURI_LOG_D(TAG, "No protocol prefix given, assuming http");
        furi_string_set(url, HTTP_PREFIX);
        furi_string_cat(url, src_url);
    } else {
        furi_string_set(url, src_url);
    }
}

static TlsClientCertType fetch_cli_get_client_cert_type(const char* str_type) {
    TlsClientCertType client_cert_type;

    if(strcmp(str_type, "none") == 0) {
        client_cert_type = TlsClientCertTypeNone;
    } else if(strcmp(str_type, "device") == 0) {
        client_cert_type = TlsClientCertTypeDevice;
    } else if(strcmp(str_type, "cert") == 0) {
        client_cert_type = TlsClientCertTypeCustom;
    } else {
        client_cert_type = TlsClientCertTypeInvalid;
    }

    return client_cert_type;
}

static void fetch_cli_option_callback(char opt, const char* optarg, void* context) {
    furi_assert(context);
    FetchCliParams* params = context;

    FetchRequest* request = &params->request;

    TlsConfig* tls_config = &request->tls_config;
    TlsClientCertInfo* client_cert_info = &tls_config->client_cert_info;

    if(opt == '\0') {
        fetch_adjust_url(params->url_store, optarg);
        request->url = furi_string_get_cstr(params->url_store);
    } else if(opt == 'H') {
        if(request->headers.count < FETCH_HEADERS_COUNT_MAX) {
            request->headers.data[request->headers.count++] = optarg;
        }
    } else if(opt == 'X') {
        request->method = optarg;
    } else if(opt == 'd') {
        request->body.data = optarg;
        request->body.length = strlen(optarg);
    } else if(opt == 'k') {
        tls_config->is_server_cert_ignored = true;
    } else if(opt == 'a') {
        client_cert_info->type = fetch_cli_get_client_cert_type(optarg);
    } else if(opt == 'C') {
        client_cert_info->paths.certificate = optarg;
    } else if(opt == 'K') {
        client_cert_info->paths.private_key = optarg;
    } else if(opt == 'o') {
        params->output_path = optarg;
    } else if(opt == 'v') {
        params->is_full_output = true;
    }
}

static FetchCliParams* fetch_cli_params_alloc(void) {
    FetchCliParams* params = malloc(sizeof(FetchCliParams));

    params->url_store = furi_string_alloc();
    tls_config_init(&params->request.tls_config);

    return params;
}

static void fetch_cli_params_free(FetchCliParams* params) {
    furi_string_free(params->url_store);
    free(params);
}

static bool fetch_cli_tls_config_validate(const TlsConfig* tls_config) {
    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);

    do {
        const TlsConfigValidationStatus tls_status = tls_config_validate(tls_config);

        if(tls_status != TlsConfigValidationStatusOk) {
            if(tls_status == TlsConfigValidationStatusInvalidType) {
                fetch_cli_print_error("Invalid client auth type");
            } else if(tls_status == TlsConfigValidationStatusClientCertNotSpecified) {
                fetch_cli_print_error("No certificate file specified");
            } else if(tls_status == TlsConfigValidationStatusPrivateKeyNotSpecified) {
                fetch_cli_print_error("No private key file specified");
            } else {
                fetch_cli_print_error("Unknown TLS error");
            }

            break;
        }

        const TlsClientCertInfo* client_cert_info = &tls_config->client_cert_info;

        if(client_cert_info->type == TlsClientCertTypeCustom) {
            const TlsClientCertPaths* paths = &client_cert_info->paths;

            if(storage_common_stat(storage, paths->certificate, NULL) != FSE_OK) {
                fetch_cli_print_error("Certificate file does not exist");
                break;
            }

            if(storage_common_stat(storage, paths->private_key, NULL) != FSE_OK) {
                fetch_cli_print_error("Private key file does not exist");
                break;
            }
        }

        success = true;

    } while(false);

    furi_record_close(RECORD_STORAGE);

    return success;
}

static bool fetch_cli_params_validate(const FetchCliParams* params) {
    bool is_valid = false;

    do {
        if(params->request.url == NULL) {
            fetch_cli_print_error("No URL specified");
            break;
        }

        if(!fetch_cli_tls_config_validate(&params->request.tls_config)) {
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

    FetchCliParams* params = fetch_cli_params_alloc();

    do {
        if(!parse_args(args, "o:d:H:X:a:C:K:kv", fetch_cli_option_callback, params)) {
            fetch_cli_print_error("Invalid arguments");
            break;
        }
        if(!fetch_cli_params_validate(params)) {
            break;
        }

        fetch_cli_run(params);
        success = true;

    } while(false);

    if(!success) {
        fetch_cli_print_usage();
    }

    fetch_cli_params_free(params);
}
