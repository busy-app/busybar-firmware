#include <furi.h>

#include <mongoose.h>
#include <usb_network/usb_network.h>
#include <wifi/wifi.h>
#include <network/network.h>
#include <mongoose_dns.h>

#define TAG "TimeTest"

typedef struct {
    Network* network;
    struct mg_mgr mgr;
    bool done;
} TimeTestApp;

static void time_test_client_callback(struct mg_connection* c, int ev, void* ev_data) {
    TimeTestApp* instance = c->fn_data;

    if(ev == MG_EV_TIME_TIME) {
        const time_t time = *(time_t*)ev_data / 1000; // Get rid of milliseconds

        char tmp[32];
        strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", gmtime(&time));

        FURI_LOG_I(TAG, "Exact UTC time: %s", tmp);

    } else if(ev == MG_EV_CLOSE) {
        instance->done = true;
    }
}

static TimeTestApp* time_test_alloc(void) {
    TimeTestApp* instance = malloc(sizeof(TimeTestApp));

    instance->network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(instance->network);

    mg_mgr_init(&instance->mgr);
    mongoose_dns_init(&instance->mgr);
    mg_time_connect(&instance->mgr, NULL, time_test_client_callback, instance);

    return instance;
}

static void time_test_free(TimeTestApp* instance) {
    mongoose_dns_deinit(&instance->mgr);
    mg_mgr_free(&instance->mgr);

    network_deinit_current_thread(instance->network);
    furi_record_close(RECORD_NETWORK);

    free(instance);
}

int time_test_app(void* arg) {
    UNUSED(arg);

    TimeTestApp* instance = time_test_alloc();

    while(!instance->done) {
        mg_mgr_poll(&instance->mgr, 1000);
    }

    time_test_free(instance);
    return 0;
}
