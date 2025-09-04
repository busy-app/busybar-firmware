#include "curl_client.h"

#include <furi.h>

#include <mongoose.h>
#include <mongoose_glue.h>
#include <wifi/wifi.h>
#include <network/network.h>

#define TAG "CurlClient"

#define CURL_CLIENT_CA_BUNDLE_PATH "/ext/ca_bundle.crt"
#define CURL_CLIENT_USER_AGENT \
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36"
#define CURL_CLIENT_THREAD_STACK_SIZE (1024 * 10) // 10 KB

#define CURL_CLIENT_DEBUG

#ifdef CURL_CLIENT_DEBUG
#define CURL_CLIENT_INFO(...)  FURI_LOG_I(__VA_ARGS__)
#define CURL_CLIENT_ERROR(...) FURI_LOG_E(__VA_ARGS__)
#else
#define CURL_CLIENT_INFO(...)
#define CURL_CLIENT_ERROR(...)
#endif

struct CurlClient {
    FuriThread* thread;
    Network* network;
    struct mg_mgr mgr;
    bool done;
    FuriString* url;
    int event;
    FuriString* response;
    uint8_t* data_body;
    size_t data_body_size;
};

static void curl_client_mg_handler(struct mg_connection* connection, int event, void* event_data) {
    UNUSED(event_data);

    CurlClient* instance = connection->fn_data;

    if(event == MG_EV_CONNECT) {
        const struct mg_str name = mg_url_host(furi_string_get_cstr(instance->url));

        if(mg_url_is_ssl(furi_string_get_cstr(instance->url))) {
            struct mg_str ca_data =
                mg_file_read((struct mg_fs*)http_fs_get(), CURL_CLIENT_CA_BUNDLE_PATH);
            const struct mg_tls_opts opts = {.ca = ca_data, .name = name};
            mg_tls_init(connection, &opts);
            free(ca_data.buf);
        }

        mg_printf(
            connection,
            "GET %s HTTP/1.0\r\nHost: %.*s\r\nUser-Agent: %s\r\n\r\n",
            mg_url_uri(furi_string_get_cstr(instance->url)),
            name.len,
            name.buf,
            CURL_CLIENT_USER_AGENT);

    } else if(event == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = event_data;
        CURL_CLIENT_INFO(TAG, "Data received: %.*s", (int)hm->message.len, hm->message.buf);

        furi_string_printf(instance->response, "%.*s", (int)hm->head.len, hm->head.buf);
        instance->event = event;

        if(hm->body.len) {
            instance->data_body = malloc(hm->body.len + 1);
            instance->data_body_size = hm->body.len;
            memcpy(instance->data_body, hm->body.buf, hm->body.len);
#ifdef CURL_CLIENT_DEBUG
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
        CURL_CLIENT_ERROR(TAG, "Error occurred: %s", (char*)event_data);

        furi_string_printf(instance->response, "Error occurred: %s", (char*)event_data);
        instance->event = event;

        instance->done = true;
    } else if(event == MG_EV_TLS_HS) {
        CURL_CLIENT_INFO(TAG, "TLS handshake successful");
    }
}

//########## Tread callbacks ##########
static void
    curl_client_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    furi_assert(thread);
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
        CURL_CLIENT_INFO(TAG, "Stop");
    }
}

static int32_t curl_client_thread_callback(void* context) {
    furi_assert(context);
    CurlClient* instance = context;
    CURL_CLIENT_INFO(TAG, "Start");

    instance->network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(instance->network);
#ifdef CURL_CLIENT_DEBUG
    mg_log_set(MG_LL_VERBOSE);
#endif
    mg_mgr_init(&instance->mgr);
    mg_http_connect(
        &instance->mgr, furi_string_get_cstr(instance->url), curl_client_mg_handler, instance);

    while(!instance->done) {
        mg_mgr_poll(&instance->mgr, 1000);
    }

    mg_mgr_free(&instance->mgr);

    network_deinit_current_thread(instance->network);
    furi_record_close(RECORD_NETWORK);

    CURL_CLIENT_INFO(TAG, "Stopping thread");

    return 0;
}

CurlClient* curl_client_alloc(FuriString* url) {
    CurlClient* instance = malloc(sizeof(CurlClient));
    instance->url = furi_string_alloc();
    instance->response = furi_string_alloc();

    furi_string_set(instance->url, url);

    return instance;
}

void curl_client_free(CurlClient* instance) {
    furi_check(instance);

    furi_string_free(instance->url);
    furi_string_free(instance->response);
    if(instance->data_body) {
        free(instance->data_body);
        instance->data_body = NULL;
    }
    free(instance);
}

void curl_client_run(CurlClient* instance) {
    furi_check(instance);

    instance->thread = furi_thread_alloc_ex(
        "CurlClient", CURL_CLIENT_THREAD_STACK_SIZE, curl_client_thread_callback, instance);
    furi_thread_set_state_callback(instance->thread, curl_client_thread_state_callback);
    CURL_CLIENT_INFO(TAG, "Starting thread");

    furi_thread_start(instance->thread);
}

bool curl_client_is_done(CurlClient* instance) {
    furi_check(instance);
    return instance->done;
}
