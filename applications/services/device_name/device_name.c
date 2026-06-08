#include "device_name_i.h"

#include <cjson/cJSON.h>

#define DEVICE_NAME_MQTT_PREFIX "state"
#define DEVICE_NAME_KEY         "name"

typedef void (*DeviceNameMessageHandler)(DeviceName* instance, const DeviceNameMessage* message);

static bool device_name_validate_char(char c) {
    static const char* const allowed_special_chars = " !()-_=+;:,.?'|@#$%^&*[]{}/\\\"<>";

    bool allowed_ascii = isalnum(c) || strchr(allowed_special_chars, c);
    bool utf8 = c >= 128;
    bool null = c == 0;
    return allowed_ascii && !utf8 && !null;
}

DeviceNameError device_name_validate(const char* name) {
    furi_assert(name);

    if(strnlen(name, DEVICE_NAME_MAX_SIZE) == 0) {
        return DeviceNameErrorEmpty;
    }

    bool only_contains_spaces = true;

    for(size_t i = 0; i < strlen(name); i++) {
        char c = name[i];

        if(c != ' ') only_contains_spaces = false;

        if(!device_name_validate_char(c)) {
            return DeviceNameErrorIllegalChar;
        }
    }

    if(only_contains_spaces) {
        return DeviceNameErrorOnlySpaces;
    }

    if(strnlen(name, DEVICE_NAME_MAX_SIZE) > DEVICE_NAME_MAX_LENGTH) {
        return DeviceNameErrorTooLong;
    }

    return DeviceNameErrorNone;
}

static void device_name_publish_mqtt_message(DeviceName* instance) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "name", instance->settings.name);
    char* json_text = cJSON_PrintUnformatted(json);
    furi_check(json_text);

    cJSON_Delete(json);

    mqtt_publish(
        instance->mqtt, MqttQosAtLeastOnce, DEVICE_NAME_MQTT_PREFIX, json_text, strlen(json_text));
    free(json_text);
}

static void device_name_publish_pubsub_event(DeviceName* instance) {
    DeviceNameEvent event = {
        .type = DeviceNameEventTypeNameChanged,
        .name_changed.name = instance->settings.name,
    };
    furi_pubsub_publish(instance->pubsub, &event);
}

static void device_name_set_handler(DeviceName* instance, const DeviceNameMessage* message) {
    const DeviceNameMessageSetName* set_name_message = &message->data.set_name;
    furi_assert(set_name_message->name);
    furi_assert(set_name_message->error);

    DeviceNameError error = DeviceNameErrorNone;

    do {
        error = device_name_validate(furi_string_get_cstr(set_name_message->name));
        if(error != DeviceNameErrorNone) break;

        snprintf(
            instance->settings.name,
            sizeof(instance->settings.name),
            furi_string_get_cstr(set_name_message->name));

        if(!device_name_settings_save(&instance->settings)) {
            error = DeviceNameErrorSaveFailed;
            break;
        }
        FURI_LOG_I(TAG, "New name: %s", furi_string_get_cstr(set_name_message->name));

        device_name_publish_mqtt_message(instance);

        device_name_publish_pubsub_event(instance);

    } while(false);

    *set_name_message->error = error;
}

static void
    device_name_publish_name_handler(DeviceName* instance, const DeviceNameMessage* message) {
    UNUSED(message);

    device_name_publish_mqtt_message(instance);
}

static void device_name_get_handler(DeviceName* instance, const DeviceNameMessage* message) {
    furi_string_set_str(message->data.get_name.name, instance->settings.name);
}

static const DeviceNameMessageHandler device_name_handlers[DeviceNameMessageTypeMax] = {
    [DeviceNameMessageTypeGetName] = device_name_get_handler,
    [DeviceNameMessageTypeSetName] = device_name_set_handler,
    [DeviceNameMessageTypeMqttPublish] = device_name_publish_name_handler,
};

static void device_name_message_queue_callback(FuriEventLoopObject* object, void* context) {
    DeviceName* instance = context;
    furi_assert(object == instance->queue);

    DeviceNameMessage message = {};
    furi_check(furi_message_queue_get(instance->queue, &message, FuriWaitForever) == FuriStatusOk);
    furi_assert(message.type < DeviceNameMessageTypeMax);

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
            .type = DeviceNameMessageTypeMqttPublish,
        };
        furi_check(
            furi_message_queue_put(instance->queue, &message, FuriWaitForever) == FuriStatusOk);
    }
}

static DeviceName* device_name_alloc(void) {
    DeviceName* instance = malloc(sizeof(DeviceName));

    instance->event_loop = furi_event_loop_alloc();
    instance->queue = furi_message_queue_alloc(1, sizeof(DeviceNameMessage));
    instance->pubsub = furi_pubsub_alloc();

    device_name_settings_load(&instance->settings);
    FURI_LOG_I(TAG, "Device name: %s", instance->settings.name);

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

    furi_record_create(RECORD_DEVICE_NAME, instance);

    return instance;
}

int32_t device_name_srv(void* arg) {
    UNUSED(arg);

    DeviceName* instance = device_name_alloc();

    furi_event_loop_run(instance->event_loop);

    return 0;
}
