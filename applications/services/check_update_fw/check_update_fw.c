#include "check_update_fw.h"
#include <wifi/wifi.h>
#include "helpers/check_update.h"

#define TAG "CheckUpdateFwSvc"

#define CHECK_UPDATE_FW_INTERVAL_MINUTES_TO_MS(x) ((x) * 60 * 1000)
#define CHECK_UPDATE_FW_REBOOT_INTERVAL_MINUTES \
    5000 //CHECK_UPDATE_FW_INTERVAL_MINUTES_TO_MS(5) // 5 minutes
#define CHECK_UPDATE_FW_INTERVAL_MINUTES \
    5000 //CHECK_UPDATE_FW_INTERVAL_MINUTES_TO_MS(180) // 3 hour

typedef enum {
    CheckUpdateFwStatusIdle = 0,
    CheckUpdateFwStatusSuccess,
    CheckUpdateFwStatusError,
    CheckUpdateFwStatusNotConnected,
    CheckUpdateFwStatusTimeout,
} CheckUpdateFwStatus;

typedef enum {
    CheckUpdateFwCustomEventSuccess = (1 << 0),
} CheckUpdateFwCustomEvent;

struct CheckUpdateFw {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    CheckUpdateFwStatus status;
};

static bool check_update_fw_check_wifi_connected(CheckUpdateFw* instance) {
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

static void check_update_fw_timer_callback(void* context) {
    CheckUpdateFw* instance = context;

    switch(instance->status) {
    case CheckUpdateFwStatusIdle:
        if(check_update_fw_check_wifi_connected(instance)) {
            FURI_LOG_D(TAG, "Update started");
            check_update_startup(instance);
        }
        break;
    case CheckUpdateFwStatusSuccess:
        instance->status = CheckUpdateFwStatusIdle;
        break;
    default:
        FURI_LOG_E(TAG, "Update failed with status: %d", instance->status);
        instance->status = CheckUpdateFwStatusIdle;
        break;
    }
}

void check_update_fw_status_update(CheckUpdateFw* instance, bool success) {
    furi_assert(instance);
    instance->status = success ? CheckUpdateFwStatusSuccess : CheckUpdateFwStatusError;
    furi_event_loop_set_custom_event(
        instance->event_loop,
        CheckUpdateFwCustomEventSuccess); // Reset status to idle after update
}

static void check_update_fw_custom_event_callback(uint32_t events, void* context) {
    CheckUpdateFw* instance = context;

    if(events & CheckUpdateFwCustomEventSuccess) {
        FURI_LOG_I(TAG, "Update successful");
        instance->status = CheckUpdateFwStatusIdle;
        furi_event_loop_timer_start(instance->timer, CHECK_UPDATE_FW_INTERVAL_MINUTES);
    }
}

CheckUpdateFw* check_update_fw_alloc() {
    CheckUpdateFw* instance = malloc(sizeof(CheckUpdateFw));

    instance->status = CheckUpdateFwStatusIdle;
    instance->event_loop = furi_event_loop_alloc();
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        check_update_fw_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, check_update_fw_custom_event_callback, instance);

    furi_event_loop_timer_start(instance->timer, CHECK_UPDATE_FW_REBOOT_INTERVAL_MINUTES);

    return instance;
}

int32_t check_update_fw_srv(void* p) {
    UNUSED(p);
    CheckUpdateFw* instance = check_update_fw_alloc();

    FURI_LOG_I(TAG, "Service started");
    furi_event_loop_run(instance->event_loop);

    furi_crash();
    return 0;
}
