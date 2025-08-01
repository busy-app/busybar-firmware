#include <furi.h>

#include <mongoose.h>
#include <usb_network/usb_network.h>

#define TAG "HttpsTest"

#define HTTP_URL "https://example.com"

typedef struct {
    UsbNetwork* usbnet;
    struct mg_mgr mgr;
    bool done;
} HttpTestApp;

static void http_test_mg_handler(struct mg_connection* connection, int event, void* event_data) {
    UNUSED(event_data);

    HttpTestApp* instance = connection->fn_data;
    furi_assert(instance);

    if(event == MG_EV_CONNECT) {
        const struct mg_str name = mg_url_host(HTTP_URL);
        const struct mg_tls_opts opts = {
            .name = name,
        };

        mg_tls_init(connection, &opts);
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
    }
}

static HttpTestApp* http_test_alloc(void) {
    HttpTestApp* instance = malloc(sizeof(HttpTestApp));

    instance->usbnet = furi_record_open(RECORD_USB_NETWORK);
    usb_network_thread_init(instance->usbnet);

    mg_mgr_init(&instance->mgr);
    mg_http_connect(&instance->mgr, HTTP_URL, http_test_mg_handler, instance);

    return instance;
}

static void http_test_free(HttpTestApp* instance) {
    mg_mgr_free(&instance->mgr);

    usb_network_thread_cleanup(instance->usbnet);
    furi_record_close(RECORD_USB_NETWORK);

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
