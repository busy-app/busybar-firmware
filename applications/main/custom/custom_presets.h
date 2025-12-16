#pragma once

#include "custom.h"

typedef struct {
    StatusLightsPreset preset;
    Color color;
} CustomStatusLightsPreset;

extern const TransitionOverlayPreset custom_transitions[CustomTransitionTypeMax];

extern const CustomStatusLightsPreset custom_status_lights[CustomStatusLightsTypeMax];

extern const TimerIndicatorPreset custom_timer_indicator_presets[CustomTimerIndicatorTypeMax];
