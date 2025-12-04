#pragma once

#include "busy_i.h"

typedef struct {
    StatusLightsPreset preset;
    Color color;
} BusyStatusLightsPreset;

extern const TransitionOverlayPreset busy_transitions[BusyTransitionTypeMax];

extern const BusyStatusLightsPreset busy_status_lights[BusyStatusLightsTypeMax];

extern const TimerIndicatorPreset busy_timer_indicator_presets[BusyTimerIndicatorTypeMax];
