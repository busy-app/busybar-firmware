#include "check_update_fw.h"
#include <wifi/wifi.h>
#include "helpers/check_update.h"

#define TAG "CheckUpdateFwSvc"

#define CHECK_UPDATE_FW_INTERVAL_MINUTES_TO_MS(x) ((x) * 60 * 1000)
#define CHECK_UPDATE_FW_REBOOT_INTERVAL_MINUTES \
    10000 //CHECK_UPDATE_FW_INTERVAL_MINUTES_TO_MS(5) // 5 minutes
#define CHECK_UPDATE_FW_INTERVAL_MINUTES \
    180000 //CHECK_UPDATE_FW_INTERVAL_MINUTES_TO_MS(180) // 3 hour

typedef enum {
    CheckUpdateFwStatusIdle = 0,
    CheckUpdateFwStatusSuccess,
    CheckUpdateFwStatusError,
    CheckUpdateFwStatusNotConnected,
    CheckUpdateFwStatusTimeout,
    CheckUpdateFwStatusInProgress,
    CheckUpdateFwStatusCheckWifi,
} CheckUpdateFwStatus;

typedef enum {
    CheckUpdateFwCustomEventSuccess = (1 << 0),
} CheckUpdateFwCustomEvent;

struct CheckUpdateFw {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    CheckUpdateFwStatus status;
    CheckUpdate* check_update;
    FuriPubSub* event_pubsub;
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
        instance->status = CheckUpdateFwStatusCheckWifi;
        if(check_update_fw_check_wifi_connected(instance)) {
            FURI_LOG_W(TAG, "Update started");
            instance->status = CheckUpdateFwStatusInProgress;
            check_update_startup(instance->check_update);
        }
        break;
    case CheckUpdateFwStatusSuccess:
        instance->status = CheckUpdateFwStatusIdle;
        break;
    case CheckUpdateFwStatusInProgress:
        FURI_LOG_W(TAG, "Update in progress...");
        break;
    case CheckUpdateFwStatusCheckWifi:
        FURI_LOG_W(TAG, "Checking WiFi connection...");
        break;
    default:
        FURI_LOG_E(TAG, "Update failed with status: %d", instance->status);
        instance->status = CheckUpdateFwStatusIdle;
        break;
    }
}

void check_update_fw_status_update(CheckUpdateStatus status, void* context) {
    CheckUpdateFw* instance = context;
    furi_assert(instance);
    CheckUpdateFwEvent pub_event = {.type = CheckUpdateFwEventError};
    if(status & CheckUpdateStatusError) {
        instance->status = CheckUpdateFwStatusError;
        FURI_LOG_E(TAG, "Update error occurred");
    } else if(status & CheckUpdateStatusNoNewVersion) {
        FURI_LOG_I(TAG, "No new version available");
        pub_event.type = CheckUpdateFwEventNoNewVersion;
    } else if(status & CheckUpdateStatusNewVersion) {
        FURI_LOG_I(TAG, "New version available");
        pub_event.type = CheckUpdateFwEventNewVersion;
    }
    furi_pubsub_publish(instance->event_pubsub, &pub_event);

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

    instance->check_update = check_update_init();
    check_update_set_callback_done(
        instance->check_update, check_update_fw_status_update, instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, check_update_fw_custom_event_callback, instance);

    furi_event_loop_timer_start(instance->timer, CHECK_UPDATE_FW_REBOOT_INTERVAL_MINUTES);

    instance->event_pubsub = furi_pubsub_alloc();
    furi_record_create(RECORD_CHECK_UPDATE_FW, instance);
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

FuriPubSub* check_update_fw_get_pubsub(CheckUpdateFw* instance) {
    furi_check(instance);
    return instance->event_pubsub;
}

void check_update_fw_startup(CheckUpdateFw* instance) {
    furi_check(instance);
    check_update_fw_timer_callback(instance);
}

bool check_update_fw_is_new_version(CheckUpdateFw* instance) {
    furi_check(instance);
    UNUSED(instance);
    return check_update_is_new_version();
}

void check_update_fw_get_current_version(CheckUpdateFw* instance, FuriString* current_version) {
    furi_check(instance);
    furi_check(current_version);
    UNUSED(instance);
    check_update_get_current_version(current_version);
}

void check_update_fw_get_new_version(CheckUpdateFw* instance, FuriString* new_version) {
    furi_check(instance);
    furi_check(new_version);
    UNUSED(instance);
    check_update_get_new_version(new_version);
}

void check_update_fw_get_new_firmware_url(CheckUpdateFw* instance, FuriString* url) {
    furi_check(instance);
    furi_check(url);
    UNUSED(instance);
    check_update_get_new_firmware_url(url);
}

void check_update_fw_get_new_firmware_sha256(CheckUpdateFw* instance, FuriString* sha256) {
    furi_check(instance);
    furi_check(sha256);
    UNUSED(instance);
    check_update_get_new_firmware_sha256(sha256);
}
