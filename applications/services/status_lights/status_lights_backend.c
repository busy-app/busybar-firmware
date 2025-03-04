#include "status_lights_common.h"
#include "status_lights_preset_defs.h"

#include <furi/furi.h>
#include <furi_hal_pwm.h>

#include <intercom/intercom.h>

#define STATUS_LIGHTS_DEFAULT_PRESET StatusLightsPresetRainbowGradient

struct StatusLights {
    FuriEventLoop* event_loop;
    FuriMessageQueue* command_queue;
    FuriEventLoopTimer* timer;
    Intercom* intercom;

    StatusLightsGenericPreset* preset_instance;
    const StatusLightsPresetBase* preset_api;
};

static void status_lights_timer_callback(void* context) {
    furi_assert(context);

    StatusLights* instance = context;
    furi_check(instance->preset_instance);
    furi_check(instance->preset_api);

    Color color = {};
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
        furi_hal_pwm_set_rgb(command.color.r, command.color.g, command.color.b);
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

static void status_lights_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data);
    furi_assert(data_size == sizeof(StatusLightsCommand));
    furi_assert(context);

    StatusLights* instance = context;
    const StatusLightsCommand* command = data;

    status_lights_execute_command(instance, *command);

    furi_check(
        furi_message_queue_put(instance->command_queue, &command, FuriWaitForever) ==
        FuriStatusOk);
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

    instance->intercom = furi_record_open(RECORD_INTERCOM);
    intercom_set_rx_callback(
        instance->intercom,
        IntercomChannelStatusLights,
        status_lights_intercom_rx_callback,
        instance);

    furi_hal_pwm_start();

    StatusLightsCommand command = {
        .type = StatusLightsCommandSetPreset,
        .preset = STATUS_LIGHTS_DEFAULT_PRESET,
    };
    status_lights_execute_command(instance, command);

    return instance;
}

void status_lights_srv(void* p) {
    UNUSED(p);

    StatusLights* instance = status_lights_alloc();
    furi_record_create(RECORD_STATUS_LIGHTS, instance);

    furi_event_loop_run(instance->event_loop);
}
