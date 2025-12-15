#pragma once

#include "busy_i.h"

typedef struct {
    StatusLightsPreset preset;
    Color color;
} BusyStatusLightsPreset;

extern const TransitionOverlayPreset busy_transitions[BusyTransitionTypeMax];

extern const BusyStatusLightsPreset busy_status_lights[BusyStatusLightsTypeMax];

extern const TimerBarPreset busy_progress_bar[BusyTimerBarTypeMax];

extern const TimerIndicatorAnimSources busy_indicator_anim_sources;
