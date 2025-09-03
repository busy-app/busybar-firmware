#include "curl.h"

#include <furi.h>
#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <cli/cli_status.h>

#include <mongoose.h>
#include <mongoose_glue.h>
#include <wifi/wifi.h>
#include <network/network.h>

#define TAG "Curl"

#define CURL_CA_BUNDLE_PATH "/ext/ca_bundle.crt"
#define CURL_USER_AGENT \
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36"

//#define CURL_DEBUG

#ifdef CURL_DEBUG
#define CURL_INFO(...)  FURI_LOG_I(__VA_ARGS__)
#define CURL_ERROR(...) FURI_LOG_E(__VA_ARGS__)
#else
#define CURL_INFO(...)
#define CURL_ERROR(...)
#endif

typedef struct {
    Network* network;
    struct mg_mgr mgr;
    bool done;
    FuriString* url;
    int event;
    FuriString* response;
    uint8_t* data_body;
    size_t data_body_size;
} Curl;

static void curl_mg_handler(struct mg_connection* connection, int event, void* event_data) {
    UNUSED(event_data);

    Curl* instance = connection->fn_data;

    if(event == MG_EV_CONNECT) {
        const struct mg_str name = mg_url_host(furi_string_get_cstr(instance->url));

        if(mg_url_is_ssl(furi_string_get_cstr(instance->url))) {
            const struct mg_tls_opts opts = {
                .ca = mg_file_read(http_fs_get(), CURL_CA_BUNDLE_PATH), .name = name};
            mg_tls_init(connection, &opts);
        }

        mg_printf(
            connection,
            "GET %s HTTP/1.0\r\nHost: %.*s\r\nUser-Agent: %s\r\n\r\n",
            mg_url_uri(furi_string_get_cstr(instance->url)),
            name.len,
            name.buf,
            CURL_USER_AGENT);

    } else if(event == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = event_data;
        CURL_INFO(TAG, "Data received: %.*s", (int)hm->message.len, hm->message.buf);

        furi_string_printf(instance->response, "%.*s", (int)hm->head.len, hm->head.buf);
        instance->event = event;

        if(hm->body.len) {
            instance->data_body = malloc(hm->body.len + 1);
            instance->data_body_size = hm->body.len;
            memcpy(instance->data_body, hm->body.buf, hm->body.len);
#ifdef CURL_DEBUG
            FURI_LOG_I(TAG, "Header (%lu bytes):", (uint32_t)(hm->body.len));
            for(uint32_t i = 0; i < hm->body.len; i++) {
                FURI_LOG_RAW_I("%02X ", instance->data_body[i]);
            }
            FURI_LOG_RAW_I("\r\n");
#endif
        }

        connection->is_draining = 1;
        instance->done = true;

    } else if(event == MG_EV_ERROR) {
        CURL_ERROR(TAG, "Error occurred: %s", (char*)event_data);

        furi_string_printf(instance->response, "Error occurred: %s", (char*)event_data);
        instance->event = event;

        instance->done = true;
    } else if(event == MG_EV_TLS_HS) {
        CURL_INFO(TAG, "TLS handshake successful");
    }
}

static Curl* curl_alloc(FuriString* url) {
    Curl* instance = malloc(sizeof(Curl));
    instance->url = furi_string_alloc();
    instance->response = furi_string_alloc();

    furi_string_set(instance->url, url);

    instance->network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(instance->network);
#ifdef CURL_DEBUG
    mg_log_set(MG_LL_VERBOSE);
#endif
    mg_mgr_init(&instance->mgr);
    mg_http_connect(
        &instance->mgr, furi_string_get_cstr(instance->url), curl_mg_handler, instance);

    return instance;
}

static void curl_free(Curl* instance) {
    mg_mgr_free(&instance->mgr);

    network_deinit_current_thread(instance->network);
    furi_record_close(RECORD_NETWORK);

    furi_string_free(instance->url);
    furi_string_free(instance->response);
    if(instance->data_body) {
        free(instance->data_body);
        instance->data_body = NULL;
    }
    free(instance);
}

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

    Curl* instance = curl_alloc(url);
    const char spin[] = "|/-\\";
    uint8_t i = 0;
    while(!instance->done) {
        mg_mgr_poll(&instance->mgr, 1000);
        printf("\rWaiting... %c", spin[i]);
        fflush(stdout);
        i = (i + 1) % 4;
    }
    printf("\r");

    if(instance->event == MG_EV_HTTP_MSG) {
        printf(ANSI_FG_GREEN "Request succeeded\r\n" ANSI_RESET);
        printf("%s\r\n", furi_string_get_cstr(instance->response));
        if(instance->data_body && instance->data_body_size) {
            printf("%.*s\r\n", (int)instance->data_body_size, instance->data_body);
        }
    } else if(instance->event == MG_EV_ERROR) {
        printf(ANSI_FG_RED "Request failed\r\n" ANSI_RESET);
        printf("%s\r\n", furi_string_get_cstr(instance->response));
    }

    curl_free(instance);
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
