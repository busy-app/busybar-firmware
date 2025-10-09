#include "sntp_time_update.h"
#include "sntp.h"

#include <network/network.h>

#include <furi.h>
#include <furi_hal_rtc.h>
#include <mongoose.h>

#define TAG "SntpTimeUpdate"

#define SNTP_UPDATE_TIMEOUT_MS        30000
#define SNTP_UPDATE_THREAD_STACK_SIZE (1024 * 2)

#define SNTP_H_TO_S(x) ((x) * 60 * 60)

typedef enum {
    SntpTimeUpdateStatusNone,

    SntpTimeUpdateStatusOk,
    SntpTimeUpdateStatusError,
    SntpTimeUpdateStatusTimeout,
} SntpTimeUpdateStatus;

typedef struct {
    int timezone_offset;
    SntpTimeUpdateStatus update_status;
    bool is_update_in_progress;
} SntpTimeUpdateContext;

static void rtc_adjust_time(time_t mseconds_epoch, int timezone_offset) {
    /* extract millisecond-precision epoch time */
    time_t seconds_new = mseconds_epoch / 1000;
    int mseconds_new = mseconds_epoch % 1000;

    /* capture current RTC time before update for delta calculation */
    DateTime datetime_new, datetime_old;
    furi_hal_rtc_get_datetime(&datetime_old);

    /* apply timezone offset & update RTC with synchronized time */
    time_t timezone_offset_seconds = SNTP_H_TO_S(timezone_offset);
    datetime_timestamp_to_datetime(seconds_new + timezone_offset_seconds, &datetime_new);
    furi_hal_rtc_set_datetime(&datetime_new);

    /* log updated UTC time */
    char datetime_new_str[20]; /* 19 for "YYYY-MM-DD HH:MM:SS" + 1 for null terminator */
    strftime(
        datetime_new_str, sizeof(datetime_new_str), "%Y-%m-%d %H:%M:%S", gmtime(&seconds_new));
    FURI_LOG_I(TAG, "Exact UTC time: %s.%03d", datetime_new_str, mseconds_new);

    /* log count of seconds RTC was adjusted */
    time_t seconds_old = datetime_datetime_to_timestamp(&datetime_old) - timezone_offset_seconds;
    FURI_LOG_I(TAG, "Time adjustment from RTC: %+d seconds", (int)(seconds_new - seconds_old));
}

static void sntp_time_update_callback(struct mg_connection* c, int ev, void* ev_data) {
    SntpTimeUpdateContext* context = c->fn_data;

    switch(ev) {
    case MG_EV_SNTP_TIME: {
        rtc_adjust_time(*(time_t*)ev_data, context->timezone_offset);
        context->update_status = SntpTimeUpdateStatusOk;
        break;
    }

    case MG_EV_ERROR:
        FURI_LOG_E(TAG, "SNTP error: %s", (const char*)ev_data);
        context->update_status = SntpTimeUpdateStatusError;
        break;

    case MG_EV_CLOSE:
        context->is_update_in_progress = false;
        break;
    }
}

static int32_t sntp_time_update_thread_callback(void* context) {
    furi_assert(context);

    Sntp* instance = context;
    SntpSettings settings;
    sntp_get_settings(instance, &settings);

    SntpTimeUpdateContext update_context = {
        .timezone_offset = settings.timezone_offset,
        .update_status = SntpTimeUpdateStatusNone,
        .is_update_in_progress = true,
    };

    FURI_LOG_D(TAG, "Start");

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_sntp_connect(&mgr, settings.server_address, sntp_time_update_callback, &update_context);

    uint32_t timeout_tick = furi_get_tick() + furi_ms_to_ticks(SNTP_UPDATE_TIMEOUT_MS);
    while(update_context.is_update_in_progress) {
        mg_mgr_poll(&mgr, 1000);

        if(furi_get_tick() > timeout_tick) {
            FURI_LOG_W(TAG, "SNTP update timeout");
            update_context.update_status = SntpTimeUpdateStatusTimeout;
            break;
        }
    }

    mg_mgr_free(&mgr);
    network_deinit_current_thread(network);
    furi_record_close(RECORD_NETWORK);

    sntp_status_update(instance, update_context.update_status == SntpTimeUpdateStatusOk);

    FURI_LOG_D(TAG, "Stopping thread");

    return 0;
}

static void sntp_time_update_thread_state_callback(
    FuriThread* thread,
    FuriThreadState state,
    void* context) {
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
        FURI_LOG_D(TAG, "Stop");
    }
}

void sntp_time_update_startup(void* context) {
    furi_assert(context);

    FuriThread* time_update_thread = furi_thread_alloc_ex(
        "SntpTimeUpdate", SNTP_UPDATE_THREAD_STACK_SIZE, sntp_time_update_thread_callback, context);
    furi_thread_set_state_callback(time_update_thread, sntp_time_update_thread_state_callback);
    FURI_LOG_D(TAG, "Starting thread");

    furi_thread_start(time_update_thread);
}
