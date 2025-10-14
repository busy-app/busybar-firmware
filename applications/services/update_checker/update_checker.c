#include "update_checker.h"
#include "update_checker_i.h"
#include <wifi/wifi.h>
#include "helpers/check_update.h"
#include <json_helper.h>
#include <api_lock.h>

#define TAG "UpdateCheckerSvc"

#define UPDATE_CHECKER_MAX_MESSAGES (8)

typedef enum {
    UpdateCheckerStatusIdle = 0,
    UpdateCheckerStatusSuccess,
    UpdateCheckerStatusError,
    UpdateCheckerStatusNotConnected,
    UpdateCheckerStatusTimeout,
    UpdateCheckerStatusInProgress,
    UpdateCheckerStatusCheckWifi,
} UpdateCheckerStatus;

typedef enum {
    UpdateCheckerMessageTypeCheckUpdate,
    UpdateCheckerMessageTypeIsNewVersion,
    UpdateCheckerMessageTypeGetCurrentVersion,
    UpdateCheckerMessageTypeGetNewVersion,
    UpdateCheckerMessageTypeGetNewFirmwareUrl,
    UpdateCheckerMessageTypeGetNewFirmwareSha256,
} UpdateCheckerMessageType;

typedef struct {
    UpdateCheckerMessageType type;
    FuriApiLock lock;
    bool* result;
    union {
        FuriString* str_data;
    };
} UpdateCheckerMessage;

typedef enum {
    UpdateCheckerCustomEventSuccess = (1 << 0),
} UpdateCheckerCustomEvent;

struct UpdateChecker {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    UpdateCheckerStatus status;
    CheckUpdate* check_update;
    FuriPubSub* event_pubsub;
    FuriMessageQueue* message_queue;
    int examination_interval;
    int reboot_examination_interval;
};

static void
    update_checker_send_message(UpdateChecker* instance, const UpdateCheckerMessage* message) {
    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);

    if(message->lock) {
        api_lock_wait_unlock_and_free(message->lock);
    }
}

static bool update_checker_check_wifi_connected(UpdateChecker* instance) {
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

static void update_checker_run(UpdateChecker* instance) {
    furi_check(instance);
    switch(instance->status) {
    case UpdateCheckerStatusIdle:
        instance->status = UpdateCheckerStatusCheckWifi;
        if(update_checker_check_wifi_connected(instance)) {
            FURI_LOG_I(TAG, "Update started");
            instance->status = UpdateCheckerStatusInProgress;
            check_update_run(instance->check_update);
        } else {
            instance->status = UpdateCheckerStatusIdle;
            FURI_LOG_W(TAG, "No WiFi connection");
            UpdateCheckerEvent pub_event = {.type = UpdateCheckerEventNoWifiConnection};
            furi_pubsub_publish(instance->event_pubsub, &pub_event);
        }
        break;
    case UpdateCheckerStatusSuccess:
        instance->status = UpdateCheckerStatusIdle;
        break;
    case UpdateCheckerStatusInProgress:
        FURI_LOG_W(TAG, "Update in progress...");
        break;
    case UpdateCheckerStatusCheckWifi:
        FURI_LOG_W(TAG, "Checking WiFi connection...");
        break;
    default:
        FURI_LOG_E(TAG, "Update failed with status: %d", instance->status);
        instance->status = UpdateCheckerStatusIdle;
        break;
    }
}

static void update_checker_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    UpdateChecker* instance = context;
    furi_assert(object == instance->message_queue);

    UpdateCheckerMessage msg;
    furi_check(furi_message_queue_get(instance->message_queue, &msg, 0) == FuriStatusOk);

    bool result = false;

    switch(msg.type) {
    case UpdateCheckerMessageTypeCheckUpdate:
        update_checker_run(instance);
        break;
    case UpdateCheckerMessageTypeIsNewVersion:
        result = check_update_is_new_version(instance->check_update);
        break;
    case UpdateCheckerMessageTypeGetCurrentVersion:
        check_update_get_current_version(instance->check_update, msg.str_data);
        break;
    case UpdateCheckerMessageTypeGetNewVersion:
        check_update_get_new_version(instance->check_update, msg.str_data);
        break;
    case UpdateCheckerMessageTypeGetNewFirmwareUrl:
        check_update_get_new_firmware_url(instance->check_update, msg.str_data);
        break;
    case UpdateCheckerMessageTypeGetNewFirmwareSha256:
        check_update_get_new_firmware_sha256(instance->check_update, msg.str_data);
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

static void update_checker_timer_callback(void* context) {
    UpdateChecker* instance = context;
    update_checker_run(instance);
}

static void update_checker_load_config(UpdateChecker* instance) {
    furi_assert(instance);

    int reboot_examination_interval_default =
        UPDATE_CHECKER_JSON_REBOOT_EXAMINATION_INTERVAL_DEFAULT;
    int examination_interval_default = UPDATE_CHECKER_JSON_EXAMINATION_INTERVAL_DEFAULT;

    FuriString* json_url = furi_string_alloc_printf(UPDATE_CHECKER_JSON_URL_DEFAULT);
    FuriString* channel_id = furi_string_alloc_printf(UPDATE_CHECKER_JSON_CHANNEL_ID_DEFAULT);

    if(json_config_read_single_int(
           UPDATE_CHECKER_SETTINGS_FILE,
           UPDATE_CHECKER_JSON_REBOOT_EXAMINATION_INTERVAL,
           &instance->reboot_examination_interval,
           &reboot_examination_interval_default) == JsonConfigStatusMissing) {
        FURI_LOG_W(TAG, "No reboot examination interval found, using default");
        json_config_write_single_int(
            UPDATE_CHECKER_SETTINGS_FILE,
            UPDATE_CHECKER_JSON_REBOOT_EXAMINATION_INTERVAL,
            UPDATE_CHECKER_JSON_REBOOT_EXAMINATION_INTERVAL_DEFAULT);
    }
    if(json_config_read_single_int(
           UPDATE_CHECKER_SETTINGS_FILE,
           UPDATE_CHECKER_JSON_EXAMINATION_INTERVAL,
           &instance->examination_interval,
           &examination_interval_default) == JsonConfigStatusMissing) {
        FURI_LOG_W(TAG, "No examination interval found, using default");
        json_config_write_single_int(
            UPDATE_CHECKER_SETTINGS_FILE,
            UPDATE_CHECKER_JSON_EXAMINATION_INTERVAL,
            UPDATE_CHECKER_JSON_EXAMINATION_INTERVAL_DEFAULT);
    }

    if(json_config_read_single_str(
           UPDATE_CHECKER_SETTINGS_FILE,
           UPDATE_CHECKER_JSON_URL_DIRECTORY,
           json_url,
           UPDATE_CHECKER_JSON_URL_DEFAULT) == JsonConfigStatusMissing) {
        FURI_LOG_W(TAG, "No JSON URL found, using default");
        json_config_write_single_str(
            UPDATE_CHECKER_SETTINGS_FILE,
            UPDATE_CHECKER_JSON_URL_DIRECTORY,
            UPDATE_CHECKER_JSON_URL_DEFAULT);
    }

    if(json_config_read_single_str(
           UPDATE_CHECKER_SETTINGS_FILE,
           UPDATE_CHECKER_JSON_CURRENT_CHANNEL,
           channel_id,
           UPDATE_CHECKER_JSON_CHANNEL_ID_DEFAULT) == JsonConfigStatusMissing) {
        FURI_LOG_W(TAG, "No channel ID found, using default");
        json_config_write_single_str(
            UPDATE_CHECKER_SETTINGS_FILE,
            UPDATE_CHECKER_JSON_CURRENT_CHANNEL,
            UPDATE_CHECKER_JSON_CHANNEL_ID_DEFAULT);
    }

    check_update_set_json_url(instance->check_update, furi_string_get_cstr(json_url));
    check_update_set_channel_id(instance->check_update, furi_string_get_cstr(channel_id));
    furi_string_free(json_url);
    furi_string_free(channel_id);
}

void update_checker_status_update(CheckUpdateStatus status, void* context) {
    UpdateChecker* instance = context;
    furi_assert(instance);
    UpdateCheckerEvent pub_event = {.type = UpdateCheckerEventError};
    if(status & CheckUpdateStatusError) {
        instance->status = UpdateCheckerStatusError;
        FURI_LOG_E(TAG, "Update error occurred");
    } else if(status & CheckUpdateStatusNoNewVersion) {
        FURI_LOG_I(TAG, "No new version available");
        pub_event.type = UpdateCheckerEventNoNewVersion;
    } else if(status & CheckUpdateStatusNewVersion) {
        FURI_LOG_I(TAG, "New version available");
        pub_event.type = UpdateCheckerEventNewVersion;
    }
    furi_pubsub_publish(instance->event_pubsub, &pub_event);

    furi_event_loop_set_custom_event(instance->event_loop, UpdateCheckerCustomEventSuccess);
}

static void update_checker_custom_event_callback(uint32_t events, void* context) {
    UpdateChecker* instance = context;

    if(events & UpdateCheckerCustomEventSuccess) {
        FURI_LOG_I(TAG, "Update successful");
        instance->status = UpdateCheckerStatusIdle;
        furi_event_loop_timer_start(instance->timer, instance->examination_interval);
    }
}

UpdateChecker* update_checker_alloc() {
    UpdateChecker* instance = malloc(sizeof(UpdateChecker));

    instance->status = UpdateCheckerStatusIdle;
    instance->check_update = check_update_init();
    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue =
        furi_message_queue_alloc(UPDATE_CHECKER_MAX_MESSAGES, sizeof(UpdateCheckerMessage));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        update_checker_message_queue_callback,
        instance);

    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        update_checker_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    check_update_set_callback_done(instance->check_update, update_checker_status_update, instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, update_checker_custom_event_callback, instance);

    update_checker_load_config(instance);

    furi_event_loop_timer_start(instance->timer, instance->reboot_examination_interval);

    instance->event_pubsub = furi_pubsub_alloc();
    furi_record_create(RECORD_UPDATE_CHECKER, instance);
    return instance;
}

int32_t update_checker_srv(void* p) {
    UNUSED(p);
    UpdateChecker* instance = update_checker_alloc();

    FURI_LOG_I(TAG, "Service started");
    furi_event_loop_run(instance->event_loop);

    furi_crash();
    return 0;
}

FuriPubSub* update_checker_get_pubsub(UpdateChecker* instance) {
    furi_check(instance);
    return instance->event_pubsub;
}

void update_checker_check_update(UpdateChecker* instance) {
    furi_check(instance);

    const UpdateCheckerMessage msg = {
        .type = UpdateCheckerMessageTypeCheckUpdate,
    };

    update_checker_send_message(instance, &msg);
}

bool update_checker_is_new_version(UpdateChecker* instance) {
    furi_check(instance);

    bool result;

    const UpdateCheckerMessage msg = {
        .type = UpdateCheckerMessageTypeIsNewVersion,
        .lock = api_lock_alloc_locked(),
        .result = &result,
    };

    update_checker_send_message(instance, &msg);

    return result;
}

void update_checker_get_current_version(UpdateChecker* instance, FuriString* current_version) {
    furi_check(instance);
    furi_check(current_version);

    const UpdateCheckerMessage msg = {
        .type = UpdateCheckerMessageTypeGetCurrentVersion,
        .lock = api_lock_alloc_locked(),
        .str_data = current_version,
    };

    update_checker_send_message(instance, &msg);
}

void update_checker_get_new_version(UpdateChecker* instance, FuriString* new_version) {
    furi_check(instance);
    furi_check(new_version);

    const UpdateCheckerMessage msg = {
        .type = UpdateCheckerMessageTypeGetNewVersion,
        .lock = api_lock_alloc_locked(),
        .str_data = new_version,
    };

    update_checker_send_message(instance, &msg);
}

void update_checker_get_new_firmware_url(UpdateChecker* instance, FuriString* url) {
    furi_check(instance);
    furi_check(url);

    const UpdateCheckerMessage msg = {
        .type = UpdateCheckerMessageTypeGetNewFirmwareUrl,
        .lock = api_lock_alloc_locked(),
        .str_data = url,
    };
    update_checker_send_message(instance, &msg);
}

void update_checker_get_new_firmware_sha256(UpdateChecker* instance, FuriString* sha256) {
    furi_check(instance);
    furi_check(sha256);

    const UpdateCheckerMessage msg = {
        .type = UpdateCheckerMessageTypeGetNewFirmwareSha256,
        .lock = api_lock_alloc_locked(),
        .str_data = sha256,
    };

    update_checker_send_message(instance, &msg);
}
