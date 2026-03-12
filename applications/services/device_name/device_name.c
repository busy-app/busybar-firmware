#include "device_name_i.h"

#include <cjson/cJSON.h>

#define DEVICE_NAME_MQTT_PREFIX "status"
#define DEVICE_NAME_KEY         "name"

typedef void (*DeviceNameMessageHandler)(DeviceName* instance, const DeviceNameMessage* message);

static void device_name_publish_new_name(DeviceName* instance) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "name", instance->settings.name);
    char* json_text = cJSON_PrintUnformatted(json);
    furi_check(json_text);

    cJSON_Delete(json);

    mqtt_publish(
        instance->mqtt, MqttQosAtLeastOnce, DEVICE_NAME_MQTT_PREFIX, json_text, strlen(json_text));
    free(json_text);
}

static void device_name_get_handler(DeviceName* instance, const DeviceNameMessage* message) {
    furi_string_set(message->data.get.name, instance->settings.name);
}

static void device_name_set_handler(DeviceName* instance, const DeviceNameMessage* message) {
    const DeviceNameMessageSet* set = &message->data.set;

    bool success = false;

    do {
        // Validate using SettingsProvider's built-in validation
        if(furi_string_empty(set->name)) {
            if(set->error) furi_string_set_str(set->error, "Name is empty");
            break;
        }

        if(furi_string_size(set->name) > sizeof(instance->settings.name)) {
            if(set->error)
                furi_string_printf(
                    set->error, "Name exceeds %d characters", sizeof(instance->settings.name));
            break;
        }

        // Check for only spaces
        bool only_spaces = true;
        for(size_t i = 0; i < furi_string_size(set->name); i++) {
            char c = furi_string_get_char(set->name, i);
            if(c != ' ') only_spaces = false;

            static const char* const allowed_special_chars = " !()-_=+;:,.?'|@#$%^&*[]{}/\\\"<>";
            bool allowed_ascii = isalnum((unsigned char)c) ||
                                 strchr(allowed_special_chars, c) != NULL;
            bool is_utf8 = (unsigned char)c >= 128;

            if(!allowed_ascii || is_utf8) {
                if(set->error) furi_string_printf(set->error, "Disallowed character: %c", c);
                break;
            }
        }

        if(only_spaces) {
            if(set->error) furi_string_set_str(set->error, "Name can't consist of only spaces");
            break;
        }

        // Update settings and save
        snprintf(
            instance->settings.name,
            sizeof(instance->settings.name),
            furi_string_get_cstr(set->name));

        if(!device_name_settings_save(&instance->settings)) {
            if(set->error) furi_string_set_str(set->error, "Failed to save name");
            break;
        }

        FURI_LOG_I(TAG, "New name: %s", furi_string_get_cstr(set->name));

        device_name_publish_new_name(instance);

        // Publish rename event
        DeviceNameEvent event = {
            .name = instance->settings.name,
        };
        furi_pubsub_publish(instance->pubsub, &event);

        success = true;
    } while(false);

    *set->result = success;
}

static void
    device_name_publish_name_handler(DeviceName* instance, const DeviceNameMessage* message) {
    UNUSED(message);

    device_name_publish_new_name(instance);
}

static const DeviceNameMessageHandler device_name_handlers[DeviceNameMessageTypeMax] = {
    [DeviceNameMessageTypeGet] = device_name_get_handler,
    [DeviceNameMessageTypeSet] = device_name_set_handler,
    [DeviceNameMessageTypeMqttUpdate] = device_name_publish_name_handler,
};

static void device_name_message_queue_callback(FuriEventLoopObject* object, void* context) {
    DeviceName* instance = context;
    furi_assert(object == instance->queue);

    DeviceNameMessage message = {};
    furi_check(furi_message_queue_get(instance->queue, &message, FuriWaitForever) == FuriStatusOk);
    furi_assert(message.type < DeviceNameMessageTypeMax);

    // Dispatch to handler
    device_name_handlers[message.type](instance, &message);

    if(message.api_lock) {
        api_lock_unlock(message.api_lock);
    }
}

static void device_name_mqtt_events_pubsub_callback(const void* msg, void* context) {
    furi_assert(msg);
    furi_assert(context);

    DeviceName* instance = context;
    const MqttEvent* mqtt_event = msg;

    if((mqtt_event->type == MqttEventTypeStatusChanged) &&
       (mqtt_event->status_changed.status == MqttStatusConnectedLinked)) {
        DeviceNameMessage message = {
            .api_lock = NULL,
            .type = DeviceNameMessageTypeMqttUpdate,
        };
        furi_check(
            furi_message_queue_put(instance->queue, &message, FuriWaitForever) == FuriStatusOk);
    }
}

// ========= Allocation =========

static DeviceName* device_name_alloc(void) {
    DeviceName* instance = malloc(sizeof(DeviceName));

    instance->event_loop = furi_event_loop_alloc();
    instance->queue = furi_message_queue_alloc(1, sizeof(DeviceNameMessage));
    instance->pubsub = furi_pubsub_alloc();

    // Load settings
    device_name_settings_load(&instance->settings);

    FURI_LOG_I(TAG, "Device name: %s", instance->settings.name);

    // Register message queue with event loop
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->queue,
        FuriEventLoopEventIn,
        device_name_message_queue_callback,
        instance);

    instance->mqtt = furi_record_open(RECORD_MQTT);
    instance->mqtt_events_pubsub = mqtt_get_pubsub(instance->mqtt);
    furi_pubsub_subscribe(
        instance->mqtt_events_pubsub, device_name_mqtt_events_pubsub_callback, instance);
    UNUSED(device_name_mqtt_events_pubsub_callback);

    furi_record_create(RECORD_DEVICE_NAME, instance);

    return instance;
}

void device_name_get(DeviceName* instance, FuriString* name) {
    furi_check(instance);
    furi_check(name);

    DeviceNameMessage message = {
        .api_lock = api_lock_alloc_locked(),
        .type = DeviceNameMessageTypeGet,
        .data.get =
            {
                .name = name,
            },
    };

    furi_check(furi_message_queue_put(instance->queue, &message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(message.api_lock);
}

bool device_name_set(DeviceName* instance, FuriString* name, FuriString* error) {
    furi_check(instance);
    furi_check(name);

    bool result = false;

    DeviceNameMessage message = {
        .api_lock = api_lock_alloc_locked(),
        .type = DeviceNameMessageTypeSet,
        .data.set =
            {
                .name = name,
                .error = error,
                .result = &result,
            },
    };

    furi_check(furi_message_queue_put(instance->queue, &message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(message.api_lock);

    return result;
}

FuriPubSub* device_name_get_pubsub(DeviceName* instance) {
    furi_check(instance);
    return instance->pubsub;
}

int32_t device_name_srv(void* arg) {
    UNUSED(arg);

    DeviceName* instance = device_name_alloc();

    furi_event_loop_run(instance->event_loop);

    return 0;
}
