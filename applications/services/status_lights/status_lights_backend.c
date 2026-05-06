#include "status_lights_backend.h"
#include "status_lights_common_i.h"
#include "status_lights_preset_defs.h"

#include <furi/furi.h>
#include <furi_hal_pwm.h>

#include <math.h>

#include <intercom/intercom.h>

typedef enum {
    StatusLightsEventSyncDone = 1UL << 0,
    StatusLightsEventIntercomError = 1UL << 1,
} StatusLightsEvent;

typedef void (*CommandHandler)(StatusLights* instance, const StatusLightsCommand* command);

struct StatusLights {
    FuriEventLoop* event_loop;
    FuriMessageQueue* command_queue;
    FuriEventLoopTimer* timer;
    Intercom* intercom;

    StatusLightsGenericPreset* preset_instance;
    const StatusLightsPresetBase* preset_api;

    Color active_color;
    float brightness;
};

static const CommandHandler command_handlers[];

static float status_lights_normalize_brightness(uint8_t brightness) {
    return ((1.f / 100.f) * (float)(brightness));
}

static uint16_t status_lights_scale_component(uint8_t component, float brightness) {
    if(component == 0) {
        return 0;
    }

    const float constrained = CLAMP(brightness, 1.0f, 0.0f);

    /* tuned to provide minimal dynamic range for low-value components at 5% brightness */
    const float gamma = 1.85f;
    const float pwm_scale = powf(constrained, gamma) * (UINT16_MAX / UINT8_MAX);

    /* tuned to make LEDs light up at 1-value components */
    const float offset = 4.f;
    const float scaled = (float)component * pwm_scale + offset;

    return (uint16_t)CLAMP(scaled + 0.5f, (float)UINT16_MAX, 0.0f);
}

static void status_lights_set_output(Color color, float brightness) {
    uint16_t r = status_lights_scale_component(color.r, brightness);
    uint16_t g = status_lights_scale_component(color.g, brightness);
    uint16_t b = status_lights_scale_component(color.b, brightness);

    furi_hal_pwm_set_rgb(r, g, b);
}

static void status_lights_run_pattern(void* context) {
    furi_assert(context);

    StatusLights* instance = context;

    furi_check(instance->preset_instance);
    furi_check(instance->preset_api);

    Color base_color;
    instance->preset_api->run(instance->preset_instance, &base_color);
    instance->active_color = base_color;

    status_lights_set_output(base_color, instance->brightness);
}

static void status_lights_message_queue_callback(FuriEventLoopObject* object, void* context) {
    StatusLights* instance = context;
    furi_check(object == instance->command_queue);

    StatusLightsCommand command;
    furi_check(furi_message_queue_get(instance->command_queue, &command, 0) == FuriStatusOk);

    command_handlers[command.id](instance, &command);
}

static void status_lights_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data);
    furi_assert(data_size == sizeof(StatusLightsCommand));
    furi_assert(context);

    StatusLights* instance = context;

    furi_check(
        furi_message_queue_put(instance->command_queue, data, FuriWaitForever) == FuriStatusOk);
}

static void status_lights_intercom_state_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    StatusLights* instance = context;

    StatusLightsEvent event;
    switch(*(const IntercomStatus*)message) {
    case IntercomStatusOk:
        event = StatusLightsEventSyncDone;
        break;

    case IntercomStatusErrorSync:
    /* fall-through */
    case IntercomStatusErrorFraming:
    /* fall-through */
    case IntercomStatusErrorTimeout:
        event = StatusLightsEventIntercomError;
        break;

    default:
        return;
    }

    furi_event_loop_set_custom_event(instance->event_loop, event);
}

static void status_lights_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    StatusLights* instance = context;

    if(events & StatusLightsEventSyncDone) {
        IntercomChannel* intercom_ch = intercom_channel_open(
            instance->intercom,
            IntercomChannelIdStatusLights,
            status_lights_intercom_rx_callback,
            instance);
        // Not sending anything to the channel
        UNUSED(intercom_ch);
    }

    if(events & StatusLightsEventIntercomError) {
        if(instance->preset_instance) {
            furi_assert(instance->preset_api);

            instance->preset_api->free(instance->preset_instance);
            instance->preset_instance = NULL;
            instance->preset_api = NULL;

            furi_event_loop_timer_stop(instance->timer);
        }

        furi_hal_pwm_stop();
    }
}

static StatusLights* status_lights_alloc() {
    StatusLights* instance = malloc(sizeof(StatusLights));
    instance->preset_instance = NULL;
    instance->preset_api = NULL;
    instance->active_color = (const Color){0};
    instance->brightness = status_lights_normalize_brightness(STATUS_LIGHTS_BRIGHTNESS_DEFAULT);
    instance->event_loop = furi_event_loop_alloc();
    furi_event_loop_set_custom_event_callback(
        instance->event_loop, status_lights_event_callback, instance);
    instance->command_queue = furi_message_queue_alloc(8, sizeof(StatusLightsCommand));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->command_queue,
        FuriEventLoopEventIn,
        status_lights_message_queue_callback,
        instance);
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop, status_lights_run_pattern, FuriEventLoopTimerTypePeriodic, instance);

    instance->intercom = furi_record_open(RECORD_INTERCOM);

    FuriState* intercom_state = intercom_get_state(instance->intercom);
    furi_state_subscribe(intercom_state, status_lights_intercom_state_callback, instance);

    furi_record_create(RECORD_STATUS_LIGHTS, instance);

    return instance;
}

int32_t status_lights_srv(void* p) {
    UNUSED(p);

    StatusLights* instance = status_lights_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static void
    status_lights_do_run_preset(StatusLights* instance, const StatusLightsCommand* command) {
    furi_check(command->run_preset.preset < StatusLightsPresetsCount);

    if(instance->preset_instance) {
        furi_assert(instance->preset_api);

        instance->preset_api->free(instance->preset_instance);
        instance->preset_instance = NULL;

        furi_event_loop_timer_stop(instance->timer);
    }

    instance->preset_api = status_lights_preset_list[command->run_preset.preset];

    if(instance->preset_api) {
        furi_check(instance->preset_api->period_ms > 0);

        furi_hal_pwm_start();

        instance->preset_instance = instance->preset_api->alloc(&command->run_preset.color);

        furi_event_loop_timer_start(instance->timer, instance->preset_api->period_ms);
        status_lights_run_pattern(instance);

    } else {
        furi_hal_pwm_stop();
    }
}

static void
    status_lights_do_set_brightness(StatusLights* instance, const StatusLightsCommand* command) {
    const StatusLightsCommandSetBrightness* set_brightness = &command->set_brightness;

    const uint8_t brightness = set_brightness->brightness;
    furi_check(brightness <= STATUS_LIGHTS_BRIGHTNESS_MAX);

    instance->brightness = status_lights_normalize_brightness(brightness);

    if(instance->preset_api) {
        status_lights_set_output(instance->active_color, instance->brightness);
    }
}

void status_lights_run_preset(StatusLights* instance, StatusLightsPreset preset, Color color) {
    furi_check(instance);
    furi_check(preset < StatusLightsPresetsCount);

    StatusLightsCommand command = {
        .id = StatusLightsCommandIdRunPreset,
        .run_preset =
            {
                .preset = preset,
                .color = color,
            },
    };

    furi_check(
        furi_message_queue_put(instance->command_queue, &command, FuriWaitForever) ==
        FuriStatusOk);
}

static const CommandHandler command_handlers[] = {
    [StatusLightsCommandIdRunPreset] = status_lights_do_run_preset,
    [StatusLightsCommandIdSetBrightness] = status_lights_do_set_brightness,
};

static_assert(COUNT_OF(command_handlers) == StatusLightsCommandIdMax);
