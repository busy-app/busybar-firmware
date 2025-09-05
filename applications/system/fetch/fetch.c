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
// static void fetch_client_callback(FetchClientEvent event, FetchClientData* data, void* context) {
//     FetchClient* instance = context;
//     furi_assert(instance);

//     switch(event) {
//     case FetchClientEventRawData:
//         if(data && data->raw.data && data->raw.size) {
//             // Process raw data chunk
//             // For example, print the size of the received chunk

//             for(size_t i = 0; i < data->raw.size; i++) {
//                 if(!data->raw.data[i]) {
//                     printf(" [00] ");
//                 } else {
//                     printf("%c", data->raw.data[i]);
//                 }
//                 fflush(stdout);
//             }
//         }
//         break;
//     case FetchClientEventProgress:
//         if(data) {
//             printf(
//                 "Download progress: %zu/%zu bytes\r\n",
//                 data->progress.received_file_size,
//                 data->progress.total_file_size);
//         }
//         break;
//     case FetchClientEventDone:
//         // Handle completion event
//         printf("Fetch operation completed\r\n");
//         break;
//     default:
//         break;
//     }
// }

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

    FetchClient* instance = fetch_client_alloc(url, path);
    //fetch_client_set_callback(instance, fetch_client_callback, instance);
    fetch_client_run(instance);

    // const char spin[] = "|/-\\";
    // uint8_t i = 0;
    while(!fetch_client_is_done(instance)) {
        // printf("\rWaiting... %c", spin[i]);
        // fflush(stdout);
        // i = (i + 1) % 4;
        // furi_delay_ms(200);
    }
    // printf("\r                      \r");

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
    fetch_client_free(instance);
    furi_string_free(path);
}

static void fetch_command_print_usage(void) {
    printf("Usage:\r\n");
    printf(
        "\tfetch <url> [path]\t : url - http(s)://example.com[:port], path - local file path to save response\r\n");
}

void fetch_command(PipeSide* pipe, FuriString* args, void* context) {
    FuriString* url = furi_string_alloc();
    //FuriString* path = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, url)) {
            fetch_command_print_usage();
            break;
        } else {
            fetch_url(pipe, url, args, context);
        }
    } while(false);

    //furi_string_free(path);
    furi_string_free(url);
}
