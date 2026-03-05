#include "status_lights.h"
#include "status_lights_common_private.h"

#include <intercom/intercom.h>

#include <api_lock.h>

#define TAG "StatusLights"

#define STATUS_LIGHTS_BRIGHTNESS_MIN     (0)
#define STATUS_LIGHTS_BRIGHTNESS_MAX     (100)
#define STATUS_LIGHTS_BRIGHTNESS_DEFAULT (50)

struct StatusLights {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    IntercomChannel* intercom_ch;

    StatusLightsBrightness brightness;
};

typedef enum {
    StatusLightsMessageTypeSetBrightness,
    StatusLightsMessageTypeGetBrightness,
    StatusLightsMessageTypeSetRunPreset,

    StatusLightsMessageTypesCount,
} StatusLightsMessageType;

typedef struct {
    FuriApiLock api_lock;
    StatusLightsMessageType type;
    union {
        struct {
            StatusLightsBrightness brightness;
        } as_set_brightness;

        struct {
            StatusLightsBrightness* brightness;
        } as_get_brightness;

        struct {
            StatusLightsPreset preset;
            Color color;
        } as_run_preset;
    };
} StatusLightsMessage;

typedef void (*MessageHandler)(StatusLights* instance, StatusLightsMessage* message);

static const MessageHandler message_handlers[];

static void status_lights_send_command(StatusLights* instance, StatusLightsCommand* command) {
    size_t tx_size =
        intercom_tx(instance->intercom_ch, command, sizeof(*command), FuriWaitForever);

    furi_check(tx_size == sizeof(*command), "Failed to send data");
}

static float status_lights_brightness_to_float(StatusLightsBrightness brightness) {
    return 0.01f * brightness.val;
}

static void status_lights_do_set_brightness(StatusLights* instance, StatusLightsMessage* message) {
    instance->brightness = message->as_set_brightness.brightness;

    StatusLightsCommand command = {
        .id = StatusLightsCommandIdSetBrightness,
        .as_set_brightness =
            {
                .brightness = status_lights_brightness_to_float(instance->brightness),
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

static StatusLights* status_lights_alloc() {
    StatusLights* instance = malloc(sizeof(*instance));

    instance->brightness = (StatusLightsBrightness){STATUS_LIGHTS_BRIGHTNESS_DEFAULT};

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(8, sizeof(StatusLightsMessage));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        status_lights_message_callback,
        instance);

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom_ch =
        intercom_channel_open(intercom, IntercomChannelIdStatusLights, NULL, NULL);

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

void status_lights_set_brightness(StatusLights* instance, StatusLightsBrightness brightness) {
    furi_check(instance);

    StatusLightsMessage message = {
        .api_lock = NULL,
        .type = StatusLightsMessageTypeSetBrightness,
        .as_set_brightness =
            {
                .brightness = brightness,
            },
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &message, FuriWaitForever) ==
        FuriStatusOk);
}

StatusLightsBrightness status_lights_get_brightness(StatusLights* instance) {
    furi_check(instance);

    StatusLightsBrightness brightness;
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
};

static_assert(COUNT_OF(message_handlers) == StatusLightsMessageTypesCount);
