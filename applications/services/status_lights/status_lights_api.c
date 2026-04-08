#include "status_lights_i.h"

void status_lights_init(StatusLights* instance) {
    furi_check(instance);

    StatusLightsMessage message = {
        .type = StatusLightsMessageTypeInit,
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &message, FuriWaitForever) ==
        FuriStatusOk);
}

void status_lights_run_preset(StatusLights* instance, StatusLightsPreset preset, Color color) {
    furi_check(instance);
    furi_check(preset < StatusLightsPresetsCount);

    StatusLightsMessage message = {
        .api_lock = NULL,
        .type = StatusLightsMessageTypeRunPreset,
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
