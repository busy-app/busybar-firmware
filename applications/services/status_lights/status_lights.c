#include "status_lights.h"
#include "status_lights_common_private.h"

#include <furi/furi.h>

#include <intercom/intercom.h>

struct StatusLights {
    FuriEventLoop* event_loop;
    FuriMessageQueue* command_queue;
    Intercom* intercom;

    _Atomic uint8_t brightness;
};

static void status_lights_command_callback(FuriEventLoopObject* object, void* context) {
    StatusLights* instance = context;
    furi_check(object == instance->command_queue);

    StatusLightsCommand command;
    furi_check(
        furi_message_queue_get(instance->command_queue, &command, FuriWaitForever) ==
        FuriStatusOk);

    size_t tx_size = intercom_tx(
        instance->intercom,
        IntercomChannelStatusLights,
        &command,
        sizeof(StatusLightsCommand),
        FuriWaitForever);

    furi_check(tx_size == sizeof(StatusLightsCommand), "Failed to send data");
}

static StatusLights* status_lights_alloc() {
    StatusLights* instance = malloc(sizeof(StatusLights));
    instance->event_loop = furi_event_loop_alloc();
    instance->command_queue = furi_message_queue_alloc(8, sizeof(StatusLightsCommand));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->command_queue,
        FuriEventLoopEventIn,
        status_lights_command_callback,
        instance);
    instance->intercom = furi_record_open(RECORD_INTERCOM);

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

    StatusLightsCommand command = {
        .id = StatusLightsCommandIdRunPreset,
        .as_run_preset =
            {
                .preset = preset,
                .color = color,
            },
    };

    furi_check(
        furi_message_queue_put(instance->command_queue, &command, FuriWaitForever) ==
        FuriStatusOk);
}

void status_lights_set_brightness(StatusLights* instance, uint8_t brightness) {
    furi_check(instance);

    instance->brightness = brightness;

    StatusLightsCommand command = {
        .id = StatusLightsCommandIdSetBrightness,
        .as_set_brightness =
            {
                .brightness = brightness * 0.01f,
            },
    };

    furi_check(
        furi_message_queue_put(instance->command_queue, &command, FuriWaitForever) ==
        FuriStatusOk);
}

uint8_t status_lights_get_brightness(StatusLights* instance) {
    furi_check(instance);

    return instance->brightness;
}
