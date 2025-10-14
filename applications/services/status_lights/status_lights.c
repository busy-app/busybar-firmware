#include "status_lights.h"
#include "status_lights_common_private.h"

#include <intercom/intercom.h>
#include <light_sensor/light_sensor.h>
#include <storage/storage.h>

#include <api_lock.h>
#include <json_helper.h>

#define TAG "StatusLights"

#define STATUS_LIGHTS_CONFIG_FILE APP_DATA_PATH("config.json")

#define AUTO_BRIGHTNESS_MIN_LEVEL (5)
#define AUTO_BRIGHTNESS_MAX_LEVEL (90)

struct StatusLights {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    IntercomChannel* intercom;

    uint8_t light_level;
    uint8_t brightness;
};

typedef enum {
    StatusLightsMessageTypeSetBrightness,
    StatusLightsMessageTypeGetBrightness,
    StatusLightsMessageTypeSetRunPreset,
    StatusLightsMessageTypeLightSensorUpdate,

    StatusLightsMessageTypesCount,
} StatusLightsMessageType;

typedef struct {
    FuriApiLock api_lock;
    StatusLightsMessageType type;
    union {
        struct {
            uint8_t brightness;
            bool do_save;
        } as_set_brightness;

        struct {
            uint8_t* brightness;
        } as_get_brightness;

        struct {
            StatusLightsPreset preset;
            Color color;
        } as_run_preset;

        struct {
            uint8_t light_level;
        } as_light_sensor_update;
    };
} StatusLightsMessage;

typedef void (*MessageHandler)(StatusLights* instance, StatusLightsMessage* message);

static const MessageHandler message_handlers[];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
static inline bool status_lights_is_valid_brightness(uint8_t brightness) {
    return brightness == STATUS_LIGHTS_BRIGHTNESS_AUTO ||
           (brightness >= STATUS_LIGHTS_BRIGHTNESS_MIN &&
            brightness <= STATUS_LIGHTS_BRIGHTNESS_MAX);
}
#pragma GCC diagnostic pop

static uint8_t status_lights_light_sensor_level_to_brightness(uint8_t light_level) {
    uint8_t brightness = AUTO_BRIGHTNESS_MIN_LEVEL +
                         ((AUTO_BRIGHTNESS_MAX_LEVEL - AUTO_BRIGHTNESS_MIN_LEVEL) * light_level) /
                             LIGHT_SENSOR_LIGHT_LEVEL_MAX;
    uint8_t clamped_level =
        CLAMP(brightness, AUTO_BRIGHTNESS_MAX_LEVEL, AUTO_BRIGHTNESS_MIN_LEVEL);
    FURI_LOG_D(
        TAG,
        "Light level: %d (clamped %d), brightness: %u",
        light_level,
        clamped_level,
        brightness);
    return clamped_level;
}

static void status_lights_send_command(StatusLights* instance, StatusLightsCommand* command) {
    size_t tx_size = intercom_tx(instance->intercom, command, sizeof(*command), FuriWaitForever);

    furi_check(tx_size == sizeof(*command), "Failed to send data");
}

static void status_lights_do_set_brightness(StatusLights* instance, StatusLightsMessage* message) {
    instance->brightness = message->as_set_brightness.brightness;

    if(message->as_set_brightness.do_save) {
        json_config_write_single_int(
            STATUS_LIGHTS_CONFIG_FILE, "brightness", instance->brightness);
    }

    StatusLightsCommand command = {
        .id = StatusLightsCommandIdSetBrightness,
        .as_set_brightness =
            {
                .brightness = 0.01f * ((instance->brightness == STATUS_LIGHTS_BRIGHTNESS_AUTO) ?
                                           status_lights_light_sensor_level_to_brightness(
                                               instance->light_level) :
                                           instance->brightness),
            },
    };

    status_lights_send_command(instance, &command);
}

static void status_lights_do_get_brightness(StatusLights* instance, StatusLightsMessage* message) {
    *message->as_get_brightness.brightness = instance->brightness;
}

static void status_lights_do_run_preset(StatusLights* instance, StatusLightsMessage* message) {
    StatusLightsCommand command = {
        .id = StatusLightsCommandIdRunPreset,
        .as_run_preset =
            {
                .preset = message->as_run_preset.preset,
                .color = message->as_run_preset.color,
            },
    };

    status_lights_send_command(instance, &command);
}

static void
    status_lights_on_light_sensor_event(StatusLights* instance, StatusLightsMessage* message) {
    instance->light_level = message->as_light_sensor_update.light_level;

    if(instance->brightness == STATUS_LIGHTS_BRIGHTNESS_AUTO) {
        StatusLightsCommand command = {
            .id = StatusLightsCommandIdSetBrightness,
            .as_set_brightness =
                {
                    .brightness = 0.01f * status_lights_light_sensor_level_to_brightness(
                                              instance->light_level),
                },
        };

        status_lights_send_command(instance, &command);
    }
}

static void status_lights_message_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    StatusLights* instance = context;

    furi_assert(object == instance->message_queue);

    StatusLightsMessage message;
    furi_check(
        furi_message_queue_get(instance->message_queue, &message, FuriWaitForever) ==
        FuriStatusOk);

    message_handlers[message.type](instance, &message);

    if(message.api_lock) {
        api_lock_unlock(message.api_lock);
    }
}

static void status_lights_light_sensor_event(const void* event_message, void* context) {
    furi_assert(event_message);
    furi_assert(context);

    StatusLights* instance = context;

    const LightSensorEvent* event = event_message;
    if(event->type == LightSensorEventTypeLightLevelChanged) {
        StatusLightsMessage message = {
            .api_lock = NULL,
            .type = StatusLightsMessageTypeLightSensorUpdate,
            .as_light_sensor_update =
                {
                    .light_level = event->light_level,
                },
        };

        furi_check(
            furi_message_queue_put(instance->message_queue, &message, FuriWaitForever) ==
            FuriStatusOk);
    }
}

static StatusLights* status_lights_alloc() {
    StatusLights* instance = malloc(sizeof(*instance));

    instance->brightness = STATUS_LIGHTS_BRIGHTNESS_AUTO;
    instance->light_level = 0;

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(8, sizeof(StatusLightsMessage));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        status_lights_message_callback,
        instance);

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom =
        intercom_channel_open(intercom, IntercomChannelIdStatusLights, NULL, NULL);

#if defined(SRV_LIGHT_SENSOR)
    FuriPubSub* light_sensor_pubsub = furi_record_open(RECORD_LIGHT_SENSOR_EVENTS);
    furi_pubsub_subscribe(light_sensor_pubsub, status_lights_light_sensor_event, instance);
#else
    UNUSED(status_lights_light_sensor_event);
#endif

    int stored_brightness;
    json_config_read_single_int(
        STATUS_LIGHTS_CONFIG_FILE,
        "brightness",
        &stored_brightness,
        &(int){STATUS_LIGHTS_BRIGHTNESS_AUTO});

    status_lights_set_brightness(instance, stored_brightness);

    furi_record_create(RECORD_STATUS_LIGHTS, instance);

    return instance;
}

int32_t status_lights_srv(void* p) {
    UNUSED(p);

    StatusLights* instance = status_lights_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

void status_lights_run_preset(StatusLights* instance, StatusLightsPreset preset, Color color) {
    furi_check(instance);
    furi_check(preset < StatusLightsPresetsCount);

    StatusLightsMessage message = {
        .api_lock = NULL,
        .type = StatusLightsMessageTypeSetRunPreset,
        .as_run_preset =
            {
                .preset = preset,
                .color = color,
            },
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &message, FuriWaitForever) ==
        FuriStatusOk);
}

void status_lights_set_brightness(StatusLights* instance, uint8_t brightness) {
    furi_check(instance);
    furi_check(status_lights_is_valid_brightness(brightness));

    StatusLightsMessage message = {
        .api_lock = NULL,
        .type = StatusLightsMessageTypeSetBrightness,
        .as_set_brightness =
            {
                .brightness = brightness,
                .do_save = true,
            },
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &message, FuriWaitForever) ==
        FuriStatusOk);
}

uint8_t status_lights_get_brightness(StatusLights* instance) {
    furi_check(instance);

    uint8_t brightness;
    StatusLightsMessage message = {
        .api_lock = api_lock_alloc_locked(),
        .type = StatusLightsMessageTypeGetBrightness,
        .as_get_brightness =
            {
                .brightness = &brightness,
            },
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &message, FuriWaitForever) ==
        FuriStatusOk);

    api_lock_wait_unlock_and_free(message.api_lock);

    return brightness;
}

static const MessageHandler message_handlers[] = {
    [StatusLightsMessageTypeSetBrightness] = status_lights_do_set_brightness,
    [StatusLightsMessageTypeGetBrightness] = status_lights_do_get_brightness,
    [StatusLightsMessageTypeSetRunPreset] = status_lights_do_run_preset,
    [StatusLightsMessageTypeLightSensorUpdate] = status_lights_on_light_sensor_event,
};

static_assert(COUNT_OF(message_handlers) == StatusLightsMessageTypesCount);
