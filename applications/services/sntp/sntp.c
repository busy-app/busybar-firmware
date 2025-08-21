#include "sntp.h"
#include <wifi/wifi.h>
#include "sntp_time_update.h"

#define TAG "Sntp"

#define SNTP_INTERVAL_MINUTES_TO_MS(x) ((x) * 60 * 1000)
#define SNTP_REBOOT_INTERVAL_MINUTES   SNTP_INTERVAL_MINUTES_TO_MS(5) // 5 minutes
#define SNTP_INTERVAL_MINUTES          SNTP_INTERVAL_MINUTES_TO_MS(180) // 3 hour

typedef enum {
    SntpStatusIdle = 0,
    SntpStatusSuccess,
    SntpStatusError,
    SntpStatusNotConnected,
    SntpStatusTimeout,
} SntpStatus;

typedef enum {
    SntpCustomEventSuccess = (1 << 0),
} SntpCustomEvent;

struct Sntp {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    SntpStatus status;
};

static bool sntp_check_wifi_connected(Sntp* instance) {
    furi_assert(instance);
    UNUSED(instance);

    bool ret = false;
    Wifi* wifi = furi_record_open(RECORD_WIFI);
    WifiInfo wifi_info;
    const WifiStatus status = wifi_get_info(wifi, &wifi_info);

    if(status != WifiStatusOk) {
        FURI_LOG_D(TAG, "Failed to get Wifi info: %d", status);
    } else if(wifi_info.state != WifiStateUp) {
        FURI_LOG_D(TAG, "Wifi is not connected");
    } else {
        FURI_LOG_D(TAG, "Wifi is connected");
        ret = true;
    }
    furi_record_close(RECORD_WIFI);
    return ret;
}

static void sntp_timer_callback(void* context) {
    Sntp* instance = context;

    switch(instance->status) {
    case SntpStatusIdle:
        if(sntp_check_wifi_connected(instance)) {
            sntp_time_update_startup(instance);
        }
        break;
    case SntpStatusSuccess:
        instance->status = SntpStatusIdle;
        break;
    default:
        FURI_LOG_E(TAG, "SNTP time update failed with status: %d", instance->status);
        instance->status = SntpStatusIdle;
        break;
    }
}

void sntp_status_update(Sntp* instance, bool success) {
    furi_assert(instance);
    instance->status = success ? SntpStatusSuccess : SntpStatusError;
    furi_event_loop_set_custom_event(
        instance->event_loop, SntpCustomEventSuccess); // Reset status to idle after update
}

static void sntp_custom_event_callback(uint32_t events, void* context) {
    Sntp* instance = context;

    if(events & SntpCustomEventSuccess) {
        FURI_LOG_I(TAG, "SNTP time update successful");
        instance->status = SntpStatusIdle;
        furi_event_loop_timer_start(instance->timer, SNTP_INTERVAL_MINUTES);
    }
}

Sntp* sntp_alloc() {
    Sntp* instance = malloc(sizeof(Sntp));

    instance->status = SntpStatusIdle;
    instance->event_loop = furi_event_loop_alloc();
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop, sntp_timer_callback, FuriEventLoopTimerTypePeriodic, instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, sntp_custom_event_callback, instance);

    furi_event_loop_timer_start(instance->timer, SNTP_REBOOT_INTERVAL_MINUTES);

    return instance;
}

int32_t sntp_srv(void* p) {
    UNUSED(p);
    Sntp* instance = sntp_alloc();

    FURI_LOG_I(TAG, "Service started");
    furi_event_loop_run(instance->event_loop);

    furi_crash();
    return 0;
}
