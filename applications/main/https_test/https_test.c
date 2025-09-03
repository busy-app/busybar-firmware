#include <furi.h>

#include <mongoose.h>
#include <wifi/wifi.h>
#include <network/network.h>
#include <mongoose_glue.h>

#define TAG "HttpsTest"

//#define HTTP_URL "https://www.example.com/"
#define HTTP_URL "https://www.example.com/"
//#define HTTP_URL "https://www.example.org/"
//#define HTTP_URL "https://192.168.10.2/"
//#define HTTP_URL "https://portal6400.ru/"
//#define HTTP_URL "https://letsencrypt.org/"
//#define HTTP_URL "https://gmail.com/"
//#define HTTP_URL "https://ya.ru/"
//#define HTTP_URL "https://mail.ru/"

typedef struct {
    Network* network;
    struct mg_mgr mgr;
    bool done;
} HttpTestApp;

static void http_test_mg_handler(struct mg_connection* connection, int event, void* event_data) {
    UNUSED(event_data);

    HttpTestApp* instance = connection->fn_data;
    furi_assert(instance);

    if(event == MG_EV_CONNECT) {
        const struct mg_str name = mg_url_host(HTTP_URL);
        //const struct mg_tls_opts opts = {.ca = mg_str_s(s_ca2), .name = name};

        if(mg_url_is_ssl(HTTP_URL)) {
            const struct mg_tls_opts opts = {
                .ca = mg_file_read(http_fs_get(), "/ext/ca_bundle.crt"),
                .name = name};
            mg_tls_init(connection, &opts);
        }

        mg_printf(
            connection,
            "GET %s HTTP/1.0\r\nHost: %.*s\r\n\r\n",
            mg_url_uri(HTTP_URL),
            name.len,
            name.buf);

    } else if(event == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = event_data;
        FURI_LOG_I(TAG, "Data received: %.*s", (int)hm->message.len, hm->message.buf);

        connection->is_draining = 1;
        instance->done = true;

    } else if(event == MG_EV_ERROR) {
        FURI_LOG_E(TAG, "Error occurred: %s", (char*)event_data);
        instance->done = true;
    } else if(event == MG_EV_TLS_HS) {
        FURI_LOG_I(TAG, "TLS handshake successful");
    }
}

static HttpTestApp* http_test_alloc(void) {
    HttpTestApp* instance = malloc(sizeof(HttpTestApp));

    instance->network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(instance->network);

    mg_log_set(MG_LL_VERBOSE);

    mg_mgr_init(&instance->mgr);
    mg_http_connect(&instance->mgr, HTTP_URL, http_test_mg_handler, instance);

    return instance;
}

static void http_test_free(HttpTestApp* instance) {
    mg_mgr_free(&instance->mgr);

    network_deinit_current_thread(instance->network);
    furi_record_close(RECORD_NETWORK);

    free(instance);
}

int https_test_app(void* arg) {
    UNUSED(arg);

    HttpTestApp* instance = http_test_alloc();

    while(!instance->done) {
        mg_mgr_poll(&instance->mgr, 1000);
    }

    http_test_free(instance);
    return 0;
}
