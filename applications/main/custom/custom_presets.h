#pragma once

#include "custom.h"

typedef struct {
    StatusLightsPreset preset;
    Color color;
} CustomStatusLightsPreset;

extern const TransitionOverlayPreset custom_transitions[CustomTransitionTypeMax];

extern const CustomStatusLightsPreset custom_status_lights[CustomStatusLightsTypeMax];

extern const TimerIndicatorAnimSources custom_indicator_anim_sources;
