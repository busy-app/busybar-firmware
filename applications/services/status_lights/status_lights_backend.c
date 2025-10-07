#include "status_lights_backend.h"
#include "status_lights_common_private.h"
#include "status_lights_preset_defs.h"

#include <furi/furi.h>
#include <furi_hal_pwm.h>

#include <math.h>

#include <intercom/intercom.h>

#define DEFAULT_BRIGHTNESS 1.f

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

static uint8_t status_lights_scale_component(uint8_t component, float brightness) {
    if(component == 0) {
        return 0;
    }

    float constrained = CLAMP(brightness, 1.0f, 0.0f);

    /* gamma tuned so 5% brightness becomes the first visible step */
    const float gamma = 2.08f;
    float pwm_scale = powf(constrained, gamma);

    float scaled = (float)component * pwm_scale;

    return (uint8_t)CLAMP(scaled + 0.5f, 255.0f, 0.0f);
}

static void status_lights_apply_brightness(Color* color, float brightness) {
    furi_assert(color);

    color->r = status_lights_scale_component(color->r, brightness);
    color->g = status_lights_scale_component(color->g, brightness);
    color->b = status_lights_scale_component(color->b, brightness);
}

static void status_lights_run_pattern(void* context) {
    furi_assert(context);

    StatusLights* instance = context;

    furi_check(instance->preset_instance);
    furi_check(instance->preset_api);

    Color base_color;
    instance->preset_api->run(instance->preset_instance, &base_color);
    instance->active_color = base_color;

    Color scaled_color = base_color;
    status_lights_apply_brightness(&scaled_color, instance->brightness);

    furi_hal_pwm_set_rgb(scaled_color.r, scaled_color.g, scaled_color.b);
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

static StatusLights* status_lights_alloc() {
    StatusLights* instance = malloc(sizeof(StatusLights));
    instance->preset_instance = NULL;
    instance->preset_api = NULL;
    instance->active_color = (Color){0};
    instance->brightness = DEFAULT_BRIGHTNESS;
    instance->event_loop = furi_event_loop_alloc();
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
    intercom_set_rx_callback(
        instance->intercom,
        IntercomChannelStatusLights,
        status_lights_intercom_rx_callback,
        instance);

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
    furi_check(command->as_run_preset.preset < StatusLightsPresetsCount);

    if(instance->preset_instance) {
        furi_assert(instance->preset_api);

        instance->preset_api->free(instance->preset_instance);
        instance->preset_instance = NULL;

        furi_event_loop_timer_stop(instance->timer);
    }

    instance->preset_api = status_lights_preset_list[command->as_run_preset.preset];

    if(instance->preset_api) {
        furi_check(instance->preset_api->period_ms > 0);

        furi_hal_pwm_start();

        instance->preset_instance = instance->preset_api->alloc(&command->as_run_preset.color);

        furi_event_loop_timer_start(instance->timer, instance->preset_api->period_ms);
        status_lights_run_pattern(instance);

    } else {
        furi_hal_pwm_stop();
    }
}

static void
    status_lights_do_set_brightness(StatusLights* instance, const StatusLightsCommand* command) {
    furi_check(command->as_set_brightness.brightness >= 0.f);
    furi_check(command->as_set_brightness.brightness <= 1.f);

    instance->brightness = command->as_set_brightness.brightness;

    if(instance->preset_api) {
        Color scaled_color = instance->active_color;
        status_lights_apply_brightness(&scaled_color, instance->brightness);

        furi_hal_pwm_set_rgb(scaled_color.r, scaled_color.g, scaled_color.b);
    }
}

void status_lights_run_preset(StatusLights* instance, StatusLightsPreset preset, Color color) {
    furi_check(instance);
    furi_check(preset < StatusLightsPresetsCount);

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

static const CommandHandler command_handlers[] = {
    [StatusLightsCommandIdRunPreset] = status_lights_do_run_preset,
    [StatusLightsCommandIdSetBrightness] = status_lights_do_set_brightness,
};

static_assert(COUNT_OF(command_handlers) == StatusLightsCommandIdsCount);
