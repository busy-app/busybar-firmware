#include "check_update_fw.h"
#include "check_update_fw_i.h"
#include <wifi/wifi.h>
#include "helpers/check_update.h"
#include <json_helper.h>
#include <api_lock.h>

#define TAG "CheckUpdateFwSvc"

#define CHECK_UPDATE_FW_MAX_MESSAGES (8)

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
    CheckUpdateFwMessageTypeStartup,
    CheckUpdateFwMessageTypeIsNewVersion,
    CheckUpdateFwMessageTypeGetCurrentVersion,
    CheckUpdateFwMessageTypeGetNewVersion,
    CheckUpdateFwMessageTypeGetNewFirmwareUrl,
    CheckUpdateFwMessageTypeGetNewFirmwareSha256,
} CheckUpdateFwMessageType;

typedef struct {
    CheckUpdateFwMessageType type;
    FuriApiLock lock;
    bool* result;
    union {
        FuriString* str_data;
    };
} CheckUpdateFwMessage;

typedef enum {
    CheckUpdateFwCustomEventSuccess = (1 << 0),
} CheckUpdateFwCustomEvent;

struct CheckUpdateFw {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    CheckUpdateFwStatus status;
    CheckUpdate* check_update;
    FuriPubSub* event_pubsub;
    FuriMessageQueue* message_queue;
    int examination_interval;
};

static void
    check_update_fw_send_message(CheckUpdateFw* instance, const CheckUpdateFwMessage* message) {
    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);

    if(message->lock) {
        api_lock_wait_unlock_and_free(message->lock);
    }
}

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

static void check_update_fw_run(CheckUpdateFw* instance) {
    furi_check(instance);
    switch(instance->status) {
    case CheckUpdateFwStatusIdle:
        instance->status = CheckUpdateFwStatusCheckWifi;
        if(check_update_fw_check_wifi_connected(instance)) {
            FURI_LOG_I(TAG, "Update started");
            instance->status = CheckUpdateFwStatusInProgress;
            check_update_startup(instance->check_update);
        } else {
            instance->status = CheckUpdateFwStatusIdle;
            FURI_LOG_W(TAG, "No WiFi connection");
            CheckUpdateFwEvent pub_event = {.type = CheckUpdateFwEventNoWifiConnection};
            furi_pubsub_publish(instance->event_pubsub, &pub_event);
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

static void check_update_fw_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    CheckUpdateFw* instance = context;
    furi_assert(object == instance->message_queue);

    CheckUpdateFwMessage msg;
    furi_check(furi_message_queue_get(instance->message_queue, &msg, 0) == FuriStatusOk);

    bool result = false;

    switch(msg.type) {
    case CheckUpdateFwMessageTypeStartup:
        check_update_fw_run(instance);
        break;
    case CheckUpdateFwMessageTypeIsNewVersion:
        result = check_update_is_new_version();
        break;
    case CheckUpdateFwMessageTypeGetCurrentVersion:
        check_update_get_current_version(msg.str_data);
        break;
    case CheckUpdateFwMessageTypeGetNewVersion:
        check_update_get_new_version(msg.str_data);
        break;
    case CheckUpdateFwMessageTypeGetNewFirmwareUrl:
        check_update_get_new_firmware_url(msg.str_data);
        break;
    case CheckUpdateFwMessageTypeGetNewFirmwareSha256:
        check_update_get_new_firmware_sha256(msg.str_data);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown message type: %d", msg.type);
        furi_crash("Invalid message type");
        break;
    }

    if(msg.result) {
        *msg.result = result;
    }
    if(msg.lock) {
        api_lock_unlock(msg.lock);
    }
}

static void check_update_fw_timer_callback(void* context) {
    CheckUpdateFw* instance = context;
    check_update_fw_run(instance);
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

    furi_event_loop_set_custom_event(instance->event_loop, CheckUpdateFwCustomEventSuccess);
}

static void check_update_fw_custom_event_callback(uint32_t events, void* context) {
    CheckUpdateFw* instance = context;

    if(events & CheckUpdateFwCustomEventSuccess) {
        FURI_LOG_I(TAG, "Update successful");
        instance->status = CheckUpdateFwStatusIdle;
        furi_event_loop_timer_start(instance->timer, instance->examination_interval);
    }
}

CheckUpdateFw* check_update_fw_alloc() {
    CheckUpdateFw* instance = malloc(sizeof(CheckUpdateFw));

    instance->status = CheckUpdateFwStatusIdle;
    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue =
        furi_message_queue_alloc(CHECK_UPDATE_FW_MAX_MESSAGES, sizeof(CheckUpdateFwMessage));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        check_update_fw_message_queue_callback,
        instance);

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

    // Load config
    int reboot_examination_interval = 0;
    int reboot_examination_interval_default =
        CHECK_UPDATE_FW_JSON_REBOOT_EXAMINATION_INTERVAL_DEFAULT;
    int examination_interval_default = CHECK_UPDATE_FW_JSON_EXAMINATION_INTERVAL_DEFAULT;

    if(json_config_read_single_int(
           CHECK_UPDATE_FW_SETTINGS_FILE,
           CHECK_UPDATE_FW_JSON_REBOOT_EXAMINATION_INTERVAL,
           &reboot_examination_interval,
           &reboot_examination_interval_default) == JsonConfigStatusMissing) {
        FURI_LOG_W(TAG, "No reboot examination interval found, using default");
        json_config_write_single_int(
            CHECK_UPDATE_FW_SETTINGS_FILE,
            CHECK_UPDATE_FW_JSON_REBOOT_EXAMINATION_INTERVAL,
            CHECK_UPDATE_FW_JSON_REBOOT_EXAMINATION_INTERVAL_DEFAULT);
    }
    if(json_config_read_single_int(
           CHECK_UPDATE_FW_SETTINGS_FILE,
           CHECK_UPDATE_FW_JSON_EXAMINATION_INTERVAL,
           &instance->examination_interval,
           &examination_interval_default) == JsonConfigStatusMissing) {
        FURI_LOG_W(TAG, "No examination interval found, using default");
        json_config_write_single_int(
            CHECK_UPDATE_FW_SETTINGS_FILE,
            CHECK_UPDATE_FW_JSON_EXAMINATION_INTERVAL,
            CHECK_UPDATE_FW_JSON_EXAMINATION_INTERVAL_DEFAULT);
    }

    furi_event_loop_timer_start(instance->timer, reboot_examination_interval);

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

    const CheckUpdateFwMessage msg = {
        .type = CheckUpdateFwMessageTypeStartup,
    };

    check_update_fw_send_message(instance, &msg);
}

bool check_update_fw_is_new_version(CheckUpdateFw* instance) {
    furi_check(instance);

    bool result;

    const CheckUpdateFwMessage msg = {
        .type = CheckUpdateFwMessageTypeIsNewVersion,
        .lock = api_lock_alloc_locked(),
        .result = &result,
    };

    check_update_fw_send_message(instance, &msg);

    return result;
}

void check_update_fw_get_current_version(CheckUpdateFw* instance, FuriString* current_version) {
    furi_check(instance);
    furi_check(current_version);

    const CheckUpdateFwMessage msg = {
        .type = CheckUpdateFwMessageTypeGetCurrentVersion,
        .lock = api_lock_alloc_locked(),
        .str_data = current_version,
    };

    check_update_fw_send_message(instance, &msg);
}

void check_update_fw_get_new_version(CheckUpdateFw* instance, FuriString* new_version) {
    furi_check(instance);
    furi_check(new_version);

    const CheckUpdateFwMessage msg = {
        .type = CheckUpdateFwMessageTypeGetNewVersion,
        .lock = api_lock_alloc_locked(),
        .str_data = new_version,
    };

    check_update_fw_send_message(instance, &msg);
}

void check_update_fw_get_new_firmware_url(CheckUpdateFw* instance, FuriString* url) {
    furi_check(instance);
    furi_check(url);

    const CheckUpdateFwMessage msg = {
        .type = CheckUpdateFwMessageTypeGetNewFirmwareUrl,
        .lock = api_lock_alloc_locked(),
        .str_data = url,
    };
    check_update_fw_send_message(instance, &msg);
}

void check_update_fw_get_new_firmware_sha256(CheckUpdateFw* instance, FuriString* sha256) {
    furi_check(instance);
    furi_check(sha256);

    const CheckUpdateFwMessage msg = {
        .type = CheckUpdateFwMessageTypeGetNewFirmwareSha256,
        .lock = api_lock_alloc_locked(),
        .str_data = sha256,
    };

    check_update_fw_send_message(instance, &msg);
}
