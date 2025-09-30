#include "sntp_time_update.h"
#include "sntp.h"
#include <furi.h>
#include <network/network.h>
#include <mongoose.h>
#include <furi_hal_rtc.h>

#define TAG "SntpTimeUpdate"

#define SNTP_H_TO_S(x) ((x) * 60 * 60)

typedef enum {
    SntpTimeUpdateStatusSuccess = (1UL << 1),
    SntpTimeUpdateStatusError = (1UL << 2),
    SntpTimeUpdateStatusDone = (1UL << 3),
} SntpTimeUpdateStatus;

typedef struct {
    int timezone_offset;
    SntpTimeUpdateStatus status;
} SntpTimeUpdateContext;

static void sntp_time_update_callback(struct mg_connection* c, int ev, void* ev_data) {
    SntpTimeUpdateContext* context = c->fn_data;

    if(ev == MG_EV_SNTP_TIME) {
        const time_t time = *(time_t*)ev_data / 1000; // Get rid of milliseconds
        DateTime datetime_temp, datetime;
        furi_hal_rtc_get_datetime(&datetime_temp);

        // Update the RTC with the received time
        const time_t timezone_offset_seconds = SNTP_H_TO_S(context->timezone_offset);
        datetime_timestamp_to_datetime(time + timezone_offset_seconds, &datetime);
        furi_hal_rtc_set_datetime(&datetime);

        suseconds_t tv_usec = *(time_t*)ev_data % 1000;

        char tmp[48];
        strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", gmtime(&time));
        snprintf(tmp + strlen(tmp), sizeof(tmp) - strlen(tmp), ".%03d", (int)tv_usec);

        FURI_LOG_I(TAG, "Exact UTC time: %s", tmp);

        // Log the time adjustment from RTC
        const time_t time_temp =
            datetime_datetime_to_timestamp(&datetime_temp) - timezone_offset_seconds;
        FURI_LOG_I(TAG, "Time adjustment from RTC: %+d seconds", (int)time - (int)time_temp);

        context->status |= SntpTimeUpdateStatusSuccess;

    } else if(ev == MG_EV_CLOSE) {
        context->status |= SntpTimeUpdateStatusDone;
    }
}

static int32_t sntp_time_update_thread_callback(void* context) {
    furi_assert(context);

    Sntp* instance = context;
    const SntpSettings* settings = sntp_get_settings(instance);

    struct mg_mgr mgr;
    SntpTimeUpdateContext update_context = {
        .timezone_offset = settings->timezone_offset,
    };

    FURI_LOG_D(TAG, "Start");

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

    mg_mgr_init(&mgr);
    mg_sntp_connect(&mgr, settings->server_name, sntp_time_update_callback, &update_context);

    while(!(update_context.status & SntpTimeUpdateStatusDone)) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    network_deinit_current_thread(network);
    furi_record_close(RECORD_NETWORK);

    if(update_context.status & SntpTimeUpdateStatusSuccess) {
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
