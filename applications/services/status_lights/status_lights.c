#include "status_lights_i.h"

#include <furi_hal_pwm.h>

static void status_lights_timer_callback(void* context) {
    furi_assert(context);

    StatusLights* instance = context;
    furi_check(instance->preset_instance);
    furi_check(instance->preset_api);

    StatusLightsColor color = {};
    instance->preset_api->run(instance->preset_instance, &color);
    furi_hal_pwm_set_rgb(color.r, color.g, color.b);
}

static void status_lights_execute_command(StatusLights* instance, StatusLightsCommand command) {
    // If previous pattern was running, stop it
    if(furi_event_loop_timer_is_running(instance->timer)) {
        furi_event_loop_timer_stop(instance->timer);
        instance->preset_api->free(instance->preset_instance);
    }

    if(command.type == StatusLightsCommandSetManual) {
        furi_hal_pwm_set_rgb(command.manual.r, command.manual.g, command.manual.b);
    } else if(command.type == StatusLightsCommandSetPreset) {
        instance->preset_api = status_lights_preset_list[command.preset];
        instance->preset_instance = instance->preset_api->alloc();

        furi_check(instance->preset_api->period_ms > 0);
        furi_event_loop_timer_start(instance->timer, instance->preset_api->period_ms);
    }
}

static void status_lights_message_queue_callback(FuriEventLoopObject* object, void* context) {
    StatusLights* instance = context;
    furi_check(object == instance->command_queue);

    StatusLightsCommand command;
    furi_check(furi_message_queue_get(instance->command_queue, &command, 0) == FuriStatusOk);

    status_lights_execute_command(instance, command);
}

static StatusLights* status_lights_alloc() {
    StatusLights* instance = malloc(sizeof(StatusLights));
    instance->event_loop = furi_event_loop_alloc();
    instance->command_queue = furi_message_queue_alloc(8, sizeof(StatusLightsCommand));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->command_queue,
        FuriEventLoopEventIn,
        status_lights_message_queue_callback,
        instance);
    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        status_lights_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    furi_hal_pwm_start();

    StatusLightsCommand command = {
        .type = StatusLightsCommandSetPreset,
        .preset = StatusLightsPresetWhiteFade,
    };
    status_lights_execute_command(instance, command);

    return instance;
}

void status_lights_srv(void* p) {
    UNUSED(p);
    FURI_LOG_D(TAG, "Starting");

    StatusLights* instance = status_lights_alloc();
    furi_record_create(RECORD_STATUS_LIGHTS, instance);

    furi_event_loop_run(instance->event_loop);
}

void status_light_send_command(StatusLights* instance, StatusLightsCommand command) {
    furi_check(instance);

    furi_check(
        furi_message_queue_put(instance->command_queue, &command, FuriWaitForever) ==
        FuriStatusOk);
}
