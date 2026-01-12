#pragma once

#include "busy_i.h"

typedef struct {
    StatusLightsPreset preset;
    Color color;
} BusyStatusLightsPreset;

typedef enum {
    BusyTimerLabelTypeWork,
    BusyTimerLabelTypeRest,
    BusyTimerLabelTypeMax,
} BusyTimerLabelType;

extern const TransitionOverlayPreset busy_transitions[BusyTransitionTypeMax];

extern const BusyStatusLightsPreset busy_status_lights[BusyStatusLightsTypeMax];

extern const TimerIndicatorPreset busy_timer_indicator_presets[BusyTimerIndicatorTypeMax];

extern const TimerIndicatorTransition
    busy_timer_indicator_transitions[BusyTimerIndicatorTransitionTypeMax];

extern const TimerLabelPreset busy_timer_label_presets[BusyTimerLabelTypeMax];
