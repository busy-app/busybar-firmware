#include "status_lights_i.h"

#define STATUS_LIGHTS_API_TIMEOUT_MS (5000)

static StatusLightsStatus
    status_lights_send_api_message(StatusLights* instance, StatusLightsApiMessage* api_message) {
    StatusLightsStatus status;

    api_message->status = &status;
    api_message->lock = api_lock_alloc_locked();

    const FuriStatus queue_status = furi_message_queue_put(
        instance->message_queue, api_message, furi_ms_to_ticks(STATUS_LIGHTS_API_TIMEOUT_MS));

    if(queue_status == FuriStatusOk) {
        api_lock_wait_unlock_and_free(api_message->lock);
    } else {
        status = StatusLightsStatusTimeout;
        api_lock_free(api_message->lock);
    }

    return status;
}

void status_lights_api_unlock(StatusLightsApiMessage* api_message, StatusLightsStatus status) {
    if(api_message->lock) {
        furi_assert(api_message->status);
        *api_message->status = status;

        api_lock_unlock(api_message->lock);
    }
}

void status_lights_init(StatusLights* instance) {
    furi_check(instance);

    StatusLightsApiMessage message = {
        .type = StatusLightsApiMessageTypeInit,
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &message, FuriWaitForever) ==
        FuriStatusOk);
}

StatusLightsStatus
    status_lights_run_preset(StatusLights* instance, StatusLightsPreset preset, Color color) {
    furi_check(instance);
    furi_check(preset < StatusLightsPresetsCount);

    StatusLightsApiMessage message = {
        .type = StatusLightsApiMessageTypeRunPreset,
        .run_preset =
            {
                .preset = preset,
                .color = color,
            },
    };

    return status_lights_send_api_message(instance, &message);
}

StatusLightsStatus
    status_lights_set_brightness(StatusLights* instance, StatusLightsBrightness brightness) {
    furi_check(instance);

    StatusLightsApiMessage message = {
        .lock = NULL,
        .type = StatusLightsApiMessageTypeSetBrightness,
        .set_brightness =
            {
                .brightness = brightness,
            },
    };

    return status_lights_send_api_message(instance, &message);
}

StatusLightsStatus
    status_lights_get_brightness(StatusLights* instance, StatusLightsBrightness* brightness) {
    furi_check(instance);

    StatusLightsApiMessage message = {
        .lock = api_lock_alloc_locked(),
        .type = StatusLightsApiMessageTypeGetBrightness,
        .get_brightness =
            {
                .brightness = brightness,
            },
    };

    return status_lights_send_api_message(instance, &message);
}
