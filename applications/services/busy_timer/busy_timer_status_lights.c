#include "busy_timer_i.h"

typedef struct {
    StatusLightsPreset preset;
    Color color;
} BusyTimerStatusLightsPreset;

static const BusyTimerStatusLightsPreset status_lights_presets[BusyTimerStateMax] = {
    [BusyTimerStateIdle] =
        {
            .preset = StatusLightsPresetOff,
        },
    [BusyTimerStateWork] =
        {
            .preset = StatusLightsPresetStaticColor,
            .color = COLOR_MAKE_RGB(150, 0, 0),
        },
    [BusyTimerStateRest] =
        {
            .preset = StatusLightsPresetStaticColor,
            .color = COLOR_MAKE_RGB(10, 150, 5),
        },
};

static const BusyTimerStatusLightsPreset*
    busy_timer_status_lights_get_preset_by_timer_state(BusyTimerState timer_state) {
    furi_assert(timer_state < BusyTimerStateMax);
    return &status_lights_presets[timer_state];
}

static void busy_timer_status_lights_timer_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    BusyTimer* instance = context;

    const BusyTimerEvent* event = message;
    const BusyTimerEventType event_type = event->type;

    const BusyTimerStatusLightsPreset* preset = NULL;

    if(event_type == BusyTimerEventTypeStateChanged) {
        preset = busy_timer_status_lights_get_preset_by_timer_state(event->state_changed.state);

    } else if(event_type == BusyTimerEventTypeIntervalEnded) {
        preset = busy_timer_status_lights_get_preset_by_timer_state(BusyTimerStateIdle);

    } else if(event_type == BusyTimerEventTypePaused) {
        const BusyTimerState effective_state = event->paused.is_paused ? BusyTimerStateIdle :
                                                                         instance->state;
        preset = busy_timer_status_lights_get_preset_by_timer_state(effective_state);
    }

    if(preset) {
        status_lights_run_preset(instance->status_lights, preset->preset, preset->color);
    }
}

void busy_timer_status_lights_init(BusyTimer* instance) {
    instance->status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
    furi_pubsub_subscribe(
        instance->event_pubsub, busy_timer_status_lights_timer_pubsub_callback, instance);
}
