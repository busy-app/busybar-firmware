#include "sntp_time_update.h"
#include "sntp.h"
#include <furi.h>
#include <network/network.h>
#include <mongoose.h>
#include <furi_hal_rtc.h>

#define TAG "SntpTimeUpdate"

#define SNTP_TIME_UPDATE_SET_TIMEZONE            (4) // Set timezone offset in hours
#define SNTP_TIME_UPDATE_TIMEZONE_CALCULATION(x) ((x) * 60 * 60) // Convert hours to seconds

typedef enum {
    SntpTimeUpdateStatusSuccess = (1UL << 1),
    SntpTimeUpdateStatusError = (1UL << 2),
    SntpTimeUpdateStatusDone = (1UL << 3),
} SntpTimeUpdateStatus;

static void sntp_time_update_callback(struct mg_connection* c, int ev, void* ev_data) {
    SntpTimeUpdateStatus* sntp_time_update_done = c->fn_data;

    if(ev == MG_EV_SNTP_TIME) {
        const time_t time = *(time_t*)ev_data / 1000; // Get rid of milliseconds

        char tmp[32];
        strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", gmtime(&time));

        FURI_LOG_I(TAG, "Exact UTC time: %s", tmp);

        // Update the RTC with the received time
        DateTime datetime;
        datetime_timestamp_to_datetime(
            time + SNTP_TIME_UPDATE_TIMEZONE_CALCULATION(SNTP_TIME_UPDATE_SET_TIMEZONE),
            &datetime);
        furi_hal_rtc_set_datetime(&datetime);

        *sntp_time_update_done |= SntpTimeUpdateStatusSuccess;

    } else if(ev == MG_EV_CLOSE) {
        *sntp_time_update_done |= SntpTimeUpdateStatusDone;
    }
}

static int32_t sntp_time_update_thread_callback(void* context) {
    furi_assert(context);

    Sntp* instance = context;

    struct mg_mgr mgr;
    SntpTimeUpdateStatus status = 0;

    FURI_LOG_D(TAG, "Start");

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

    mg_mgr_init(&mgr);
    mg_sntp_connect(&mgr, NULL, sntp_time_update_callback, &status);

    while(!(status & SntpTimeUpdateStatusDone)) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);
    network_deinit_current_thread(network);
    furi_record_close(RECORD_NETWORK);

    if(status & SntpTimeUpdateStatusSuccess) {
        sntp_status_update(instance, true);
    }

    FURI_LOG_D(TAG, "Stopping thread");

    return 0;
}

static void sntp_time_update_thread_state_callback(
    FuriThread* thread,
    FuriThreadState state,
    void* context) {
    furi_assert(thread);
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
        FURI_LOG_D(TAG, "Stop");
    }
}

void sntp_time_update_startup(void* context) {
    furi_assert(context);
    FuriThread* startup_thread = furi_thread_alloc_ex(
        "SntpTimeUpdate", 1024 * 2, sntp_time_update_thread_callback, context);
    furi_thread_set_state_callback(startup_thread, sntp_time_update_thread_state_callback);
    FURI_LOG_D(TAG, "Starting thread");

    furi_thread_start(startup_thread);
}
