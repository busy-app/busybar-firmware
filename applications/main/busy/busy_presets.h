#pragma once

#include "busy.h"

typedef struct {
    StatusLightsPreset preset;
    Color color;
} BusyStatusLightsPreset;

extern const TransitionOverlayPreset busy_transitions[BusyTransitionTypeMax];

extern const BusyStatusLightsPreset busy_status_lights[BusyStatusLightsTypeMax];

extern const ProgressBarPreset busy_progress_bar[BusyProgressBarTypeMax];

extern const TimerIndicatorAnimSources busy_indicator_anim_sources;
