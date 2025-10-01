#include "sntp.h"
#include "sntp_time_update.h"

#include <wifi/wifi.h>

#define TAG "SntpSvc"

#define SNTP_S_TO_MS(x) ((x) * 1000)

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
    SntpSettings settings;
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
        furi_event_loop_timer_start(
            instance->timer, SNTP_S_TO_MS(instance->settings.auto_interval));
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

    sntp_settings_load(&instance->settings);

    furi_event_loop_timer_start(instance->timer, SNTP_S_TO_MS(instance->settings.boot_interval));

    furi_record_create(RECORD_SNTP, instance);

    return instance;
}

const SntpSettings* sntp_get_settings(const Sntp* instance) {
    furi_check(instance);
    return &instance->settings;
}

bool sntp_set_settings(Sntp* instance, const SntpSettings* settings) {
    furi_check(instance);
    furi_check(settings);

    bool is_success = sntp_settings_save(settings) && sntp_settings_load(&instance->settings);

    if(is_success) {
        if(instance->settings.is_enabled) {
            uint32_t interval = (instance->status == SntpStatusSuccess) ?
                                    instance->settings.auto_interval :
                                    instance->settings.boot_interval;

            furi_event_loop_pend_callback(instance->event_loop, sntp_timer_callback, instance);
            furi_event_loop_timer_start(instance->timer, SNTP_S_TO_MS(interval));
        } else {
            furi_event_loop_timer_stop(instance->timer);
        }
    }

    return is_success;
}

int32_t sntp_srv(void* p) {
    UNUSED(p);
    Sntp* instance = sntp_alloc();

    FURI_LOG_I(TAG, "Service started");
    furi_event_loop_run(instance->event_loop);

    furi_crash();
    return 0;
}
