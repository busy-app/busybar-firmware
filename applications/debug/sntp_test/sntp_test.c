#include <furi.h>

#include <mongoose.h>
#include <usb_network/usb_network.h>
#include <wifi/wifi.h>
#include <network/network.h>

#define TAG "SntpTest"

typedef struct {
    Network* network;
    struct mg_mgr mgr;
    bool done;
} SntpTestApp;

static void sntp_test_client_callback(struct mg_connection* c, int ev, void* ev_data) {
    SntpTestApp* instance = c->fn_data;

    if(ev == MG_EV_SNTP_TIME) {
        const time_t time = *(time_t*)ev_data / 1000; // Get rid of milliseconds

        char tmp[32];
        strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", gmtime(&time));

        FURI_LOG_I(TAG, "Exact UTC time: %s", tmp);

    } else if(ev == MG_EV_CLOSE) {
        instance->done = true;
    }
}

static SntpTestApp* sntp_test_alloc(void) {
    SntpTestApp* instance = malloc(sizeof(SntpTestApp));

    instance->network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(instance->network);

    mg_mgr_init(&instance->mgr);
    mg_sntp_connect(&instance->mgr, NULL, sntp_test_client_callback, instance);

    return instance;
}

static void sntp_test_free(SntpTestApp* instance) {
    mg_mgr_free(&instance->mgr);

    network_deinit_current_thread(instance->network);
    furi_record_close(RECORD_NETWORK);

    free(instance);
}

int sntp_test_app(void* arg) {
    UNUSED(arg);

    SntpTestApp* instance = sntp_test_alloc();

    while(!instance->done) {
        mg_mgr_poll(&instance->mgr, 1000);
    }

    sntp_test_free(instance);
    return 0;
}
