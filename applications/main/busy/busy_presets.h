#pragma once

#include "busy.h"

typedef struct {
    Color color;
    const char* mask_path;
    TransitionOverlayColorMode color_mode;
    TransitionOverlayMaskMode mask_mode;
    struct {
        uint32_t in_ms;
        uint32_t out_ms;
    } timings;
    bool enable_press;
} BusyTransition;

extern const BusyTransition busy_transitions[BusyTransitionTypeMax];

extern const StatusLightsCommand busy_status_lights[BusyStatusLightsTypeMax];

extern const TimerIndicatorAnimSources busy_indicator_anim_sources;
