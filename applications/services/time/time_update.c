#include "time_update.h"

#include <furi.h>
#include <furi_hal_rtc.h>
#include <mongoose.h>

#include <network/network.h>
#include <mongoose_dns.h>

#define TAG "TimeUpdate"

#define TIME_UPDATE_TIMEOUT_MS        30000
#define TIME_UPDATE_THREAD_STACK_SIZE (1024 * 2)

#define M_TO_S(x) ((x) * 60)

typedef enum {
    TimeTimeUpdateStatusNone,

    TimeTimeUpdateStatusOk,
    TimeTimeUpdateStatusError,
    TimeTimeUpdateStatusTimeout,
} TimeUpdateStatus;

typedef struct {
    TimeUpdateStatus update_status;
    bool is_update_in_progress;
} TimeUpdateContext;

typedef struct {
    Time* instance;
    TimeTimeUpdateCallback callback;
} ThreadContext;

static void rtc_adjust_time(time_t timestamp_ms) {
    /* capture current RTC time */
    time_t timestamp_ms_old = furi_hal_rtc_get_timestamp_ms();

    /* update RTC with received time */
    DateTimeMs datetime_new = datetime_timestamp_ms_to_datetime(timestamp_ms);
    furi_hal_rtc_set_datetime(&datetime_new);

    /* log updated UTC time */
    FURI_LOG_I(
        TAG,
        "Exact UTC time: %04" PRIu16 "-%02" PRIu8 "-%02" PRIu8 " %02" PRIu8 ":%02" PRIu8
        ":%02" PRIu8 ".%03" PRIu16,
        datetime_new.dt.year,
        datetime_new.dt.month,
        datetime_new.dt.dayofmonth,
        datetime_new.dt.hour,
        datetime_new.dt.minute,
        datetime_new.dt.second,
        datetime_new.millis);

    /* log count of seconds RTC was adjusted */
    bool is_delta_positive = timestamp_ms >= timestamp_ms_old;
    uint64_t delta_ms = llabs(timestamp_ms - timestamp_ms_old);
    FURI_LOG_I(
        TAG,
        "RTC time adjusted by %c%" PRIu64 ".%03" PRIu64 " seconds",
        is_delta_positive ? '+' : '-',
        delta_ms / 1000,
        delta_ms % 1000);
}

static void time_update_callback(struct mg_connection* c, int ev, void* ev_data) {
    TimeUpdateContext* context = c->fn_data;

    switch(ev) {
    case MG_EV_SNTP_TIME: {
        rtc_adjust_time(*(time_t*)ev_data);
        context->update_status = TimeTimeUpdateStatusOk;
        break;
    }

    case MG_EV_ERROR:
        FURI_LOG_E(TAG, "TIME error: %s", (const char*)ev_data);
        context->update_status = TimeTimeUpdateStatusError;
        break;

    case MG_EV_CLOSE:
        context->is_update_in_progress = false;
        break;
    }
}

static int32_t time_update_thread_callback(void* context) {
    ThreadContext* thread_context = context;

    FURI_LOG_D(TAG, "Thread started");

    TimeSettings settings;
    time_get_settings(thread_context->instance, &settings);

    TimeUpdateContext time_update_context = {
        .update_status = TimeTimeUpdateStatusNone,
        .is_update_in_progress = true,
    };

    Network* network = furi_record_open(RECORD_NETWORK);
    network_init_current_thread(network);

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mongoose_dns_init(&mgr);

    struct mg_connection* conn =
        mg_sntp_connect(&mgr, settings.server_address, time_update_callback, &time_update_context);

    bool do_timeout_checks = true;
    uint32_t timeout_tick = furi_get_tick() + furi_ms_to_ticks(TIME_UPDATE_TIMEOUT_MS);
    while(time_update_context.is_update_in_progress) {
        if(do_timeout_checks && furi_get_tick() > timeout_tick) {
            FURI_LOG_W(TAG, "SNTP time update timeout");

            do_timeout_checks = false;
            conn->is_draining = 1;
            time_update_context.update_status = TimeTimeUpdateStatusTimeout;
        }

        mg_mgr_poll(&mgr, 1000);
    }

    mongoose_dns_deinit(&mgr);
    mg_mgr_free(&mgr);
    network_deinit_current_thread(network);
    furi_record_close(RECORD_NETWORK);

    thread_context->callback(
        thread_context->instance, time_update_context.update_status == TimeTimeUpdateStatusOk);

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

void time_update_run(Time* instance, TimeTimeUpdateCallback callback) {
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
