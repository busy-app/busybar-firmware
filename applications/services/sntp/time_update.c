#include "time_update.h"

#include <furi.h>
#include <furi_hal_rtc.h>
#include <mongoose.h>

#include <network/network.h>

#define TAG "SntpTimeUpdate"

#define TIME_UPDATE_TIMEOUT_MS        30000
#define TIME_UPDATE_THREAD_STACK_SIZE (1024 * 2)

#define M_TO_S(x) ((x) * 60)

typedef enum {
    SntpTimeUpdateStatusNone,

    SntpTimeUpdateStatusOk,
    SntpTimeUpdateStatusError,
    SntpTimeUpdateStatusTimeout,
} TimeUpdateStatus;

typedef struct {
    TimeUpdateStatus update_status;
    bool is_update_in_progress;
} TimeUpdateContext;

typedef struct {
    Sntp* instance;
    SntpTimeUpdateCallback callback;
} ThreadContext;

static void rtc_adjust_time(time_t mseconds_epoch) {
    /* extract millisecond-precision epoch time */
    time_t seconds_new = mseconds_epoch / 1000;
    int mseconds_new = mseconds_epoch % 1000;

    /* capture current RTC time before update for delta calculation */
    DateTime datetime_new, datetime_old;
    furi_hal_rtc_get_datetime(&datetime_old);

    /* apply timezone offset & update RTC with synchronized time */
    datetime_timestamp_to_datetime(seconds_new, &datetime_new);
    furi_hal_rtc_set_datetime(&datetime_new);

    /* log updated UTC time */
    char datetime_new_str[20]; /* 19 for "YYYY-MM-DD HH:MM:SS" + 1 for null terminator */
    strftime(
        datetime_new_str, sizeof(datetime_new_str), "%Y-%m-%d %H:%M:%S", gmtime(&seconds_new));
    FURI_LOG_I(TAG, "Exact UTC time: %s.%03d", datetime_new_str, mseconds_new);

    /* log count of seconds RTC was adjusted */
    time_t seconds_old = datetime_datetime_to_timestamp(&datetime_old);
    FURI_LOG_I(TAG, "Time adjustment from RTC: %+d seconds", (int)(seconds_new - seconds_old));
}

static void time_update_callback(struct mg_connection* c, int ev, void* ev_data) {
    TimeUpdateContext* context = c->fn_data;

    switch(ev) {
    case MG_EV_SNTP_TIME: {
        rtc_adjust_time(*(time_t*)ev_data);
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

static int32_t time_update_thread_callback(void* context) {
    ThreadContext* thread_context = context;

    FURI_LOG_D(TAG, "Thread started");

    SntpSettings settings;
    sntp_get_settings(thread_context->instance, &settings);

    TimeUpdateContext time_update_context = {
        .update_status = SntpTimeUpdateStatusNone,
        .is_update_in_progress = true,
    };

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    struct mg_connection* conn =
        mg_sntp_connect(&mgr, settings.server_address, time_update_callback, &time_update_context);

    bool do_timeout_checks = true;
    uint32_t timeout_tick = furi_get_tick() + furi_ms_to_ticks(TIME_UPDATE_TIMEOUT_MS);
    while(time_update_context.is_update_in_progress) {
        if(do_timeout_checks && furi_get_tick() > timeout_tick) {
            FURI_LOG_W(TAG, "SNTP time update timeout");

            do_timeout_checks = false;
            conn->is_draining = 1;
            time_update_context.update_status = SntpTimeUpdateStatusTimeout;
        }

        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    network_deinit_current_thread(network);
    furi_record_close(RECORD_NETWORK);

    thread_context->callback(
        thread_context->instance, time_update_context.update_status == SntpTimeUpdateStatusOk);

    free(thread_context);

    FURI_LOG_D(TAG, "Stopping thread...");

    return 0;
}

static void
    time_update_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);

        FURI_LOG_D(TAG, "Thread stopped");
    }
}

void sntp_time_update_run(Sntp* instance, SntpTimeUpdateCallback callback) {
    furi_assert(instance);

    ThreadContext* thread_context = malloc(sizeof(*thread_context));
    thread_context->instance = instance;
    thread_context->callback = callback;

    FuriThread* time_update_thread = furi_thread_alloc_ex(
        TAG, TIME_UPDATE_THREAD_STACK_SIZE, time_update_thread_callback, thread_context);
    furi_thread_set_state_callback(time_update_thread, time_update_thread_state_callback);

    FURI_LOG_D(TAG, "Starting thread...");

    furi_thread_start(time_update_thread);
}
