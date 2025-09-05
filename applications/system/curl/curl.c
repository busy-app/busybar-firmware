#include "curl.h"
#include "helpers/curl_client.h"

#include <furi.h>
#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <cli/cli_status.h>

#include <mongoose.h>
#include <mongoose_glue.h>
#include <wifi/wifi.h>
#include <network/network.h>

#define TAG "Curl"

void curl_url(PipeSide* pipe, FuriString* url, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);

    size_t pos = furi_string_search_str(url, "://", 0);
    if(pos == FURI_STRING_FAILURE) {
        FuriString* url_temp = furi_string_alloc_printf("http://%s", furi_string_get_cstr(url));
        furi_string_set(url, url_temp);
        furi_string_free(url_temp);
    }
    FuriString * path = furi_string_alloc_printf("%s", "downloaded_file.bin");
    CurlClient* instance = curl_client_alloc(url, path);
    curl_client_run(instance);

    const char spin[] = "|/-\\";
    uint8_t i = 0;
    while(!curl_client_is_done(instance)) {
        printf("\rWaiting... %c", spin[i]);
        fflush(stdout);
        i = (i + 1) % 4;
        furi_delay_ms(200);
    }
    printf("\r                      \r");

    // if(instance->event == MG_EV_HTTP_MSG) {
    //     printf(ANSI_FG_GREEN "Request succeeded\r\n" ANSI_RESET);
    //     printf("%s\r\n", furi_string_get_cstr(instance->response));
    //     if(instance->data_body && instance->data_body_size) {
    //         printf("%.*s\r\n", (int)instance->data_body_size, instance->data_body);
    //     }
    // } else if(instance->event == MG_EV_ERROR) {
    //     printf(ANSI_FG_RED "Request failed\r\n" ANSI_RESET);
    //     printf("%s\r\n", furi_string_get_cstr(instance->response));
    // }
    curl_client_free(instance);
    furi_string_free(path);
}

static void curl_command_print_usage(void) {
    printf("Usage:\r\n");
    printf(
        "\tcurl <url> [path]\t : url - http(s)://example.com[:port], path - local file path to save response\r\n");
}

void curl_command(PipeSide* pipe, FuriString* args, void* context) {
    FuriString* url = furi_string_alloc();
    //FuriString* path = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, url)) {
            curl_command_print_usage();
            break;
        } else {
            curl_url(pipe, url, args, context);
        }
    } while(false);

    //furi_string_free(path);
    furi_string_free(url);
}
