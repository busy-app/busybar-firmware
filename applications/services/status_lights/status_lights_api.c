#include "status_lights_i.h"

#define STATUS_LIGHTS_API_TIMEOUT_MS (5000)

static void status_lights_api_send_message_internal(
    StatusLights* instance,
    const StatusLightsApiMessage* api_message) {
    instance->api_message = *api_message;
    furi_event_loop_set_custom_event(instance->event_loop, StatusLightsCustomEventRequest);
}

static StatusLightsStatus
    status_lights_api_send_message(StatusLights* instance, StatusLightsApiMessage* api_message) {
    StatusLightsStatus status;

    api_message->status = &status;
    api_message->lock = api_lock_alloc_locked();

    const FuriStatus sem_status = furi_semaphore_acquire(
        instance->api_semaphore, furi_ms_to_ticks(STATUS_LIGHTS_API_TIMEOUT_MS));

    if(sem_status == FuriStatusOk) {
        status_lights_api_send_message_internal(instance, api_message);
        api_lock_wait_unlock_and_free(api_message->lock);

    } else {
        FURI_LOG_E(TAG, "Request timed out");

        status = StatusLightsStatusTimeout;
        api_lock_free(api_message->lock);
    }

    return status;
}

bool status_lights_api_is_locked(StatusLights* instance) {
    return furi_semaphore_get_count(instance->api_semaphore) == 0;
}

void status_lights_api_unlock(StatusLights* instance, StatusLightsStatus status) {
    StatusLightsApiMessage* api_message = &instance->api_message;

    if(api_message->lock) {
        furi_assert(api_message->status);
        *api_message->status = status;

        api_lock_unlock(api_message->lock);
    }

    furi_check(furi_semaphore_release(instance->api_semaphore) == FuriStatusOk);
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

    return status_lights_api_send_message(instance, &message);
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

    return status_lights_api_send_message(instance, &message);
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

    return status_lights_api_send_message(instance, &message);
}
