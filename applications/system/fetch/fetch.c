#include "fetch.h"
#include "helpers/fetch_client.h"

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
} Fetch;

static void fetch_client_callback_raw_data(uint8_t* data, size_t data_size, void* context) {
    furi_assert(context);
    Fetch* instance = context;
    furi_assert(instance);
    furi_stream_buffer_send(instance->buffer_rx, data, data_size, FuriWaitForever);
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
    instance->buffer_rx = furi_stream_buffer_alloc(2048, 1);

    instance->fetch_client = fetch_client_alloc(url);
    fetch_client_set_callback_raw_data(
        instance->fetch_client, fetch_client_callback_raw_data, instance);

    fetch_client_set_callback_header(
        instance->fetch_client, fetch_client_callback_header, instance);

    fetch_client_set_callback_error(instance->fetch_client, fetch_client_callback_error, instance);

    fetch_client_run(instance->fetch_client);

    const char spin[] = "|/-\\";
    uint8_t i = 0;
    uint8_t flag_waiting = 1;
    while(!fetch_client_is_done(instance->fetch_client) ||
          furi_stream_buffer_bytes_available(instance->buffer_rx)) {
        if(!furi_stream_buffer_bytes_available(instance->buffer_rx) && flag_waiting) {
            printf("\rWaiting... %c", spin[i]);
            fflush(stdout);
            i = (i + 1) % 4;
            furi_delay_ms(200);
            continue;
        }

        if(flag_waiting) {
            printf("\r                      \r");
            flag_waiting = 0;
        }

        furi_delay_ms(200);
        while(furi_stream_buffer_bytes_available(instance->buffer_rx)) {
            uint8_t buffer[256];
            size_t bytes_read =
                furi_stream_buffer_receive(instance->buffer_rx, buffer, sizeof(buffer), 0);
            if(bytes_read > 0) {
                for(size_t i = 0; i < bytes_read; i++) {
                    if(!buffer[i]) {
                        printf(" [00] ");
                    } else {
                        printf("%c", buffer[i]);
                    }
                    fflush(stdout);
                }
            }
        }
    }
    printf("\r\n");

    fetch_client_free(instance->fetch_client);
    furi_stream_buffer_free(instance->buffer_rx);
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
