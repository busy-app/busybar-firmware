#pragma once

#include <status_lights/status_lights.h>

#include <toolbox/color.h>

#include "widgets/timer_label.h"
#include "widgets/timer_indicator.h"
#include "widgets/transition_overlay.h"

#include "busy_settings.h"

typedef enum {
    BusyTransitionTypeDefault,
    BusyTransitionTypeAutomatic,
    BusyTransitionTypeSkip,
    BusyTransitionTypeSelect,
    BusyTransitionTypeWork,
    BusyTransitionTypeRest,
    BusyTransitionTypeWorkDone,
    BusyTransitionTypeRestDone,
    BusyTransitionTypeEnding,
    BusyTransitionTypeMax,
} BusyTransitionType;

typedef enum {
    BusyStatusLightsTypeOff,
    BusyStatusLightsTypeWork,
    BusyStatusLightsTypeRest,
    BusyStatusLightsTypeMax,
} BusyStatusLightsType;

typedef enum {
    BusyTimerIndicatorTypeWork,
    BusyTimerIndicatorTypeRest,
    BusyTimerIndicatorTypeWorkBig,
    BusyTimerIndicatorTypeRestBig,
    BusyTimerIndicatorTypeMax,
} BusyTimerIndicatorType;

typedef enum {
    BusyTimerIndicatorTransitionTypeInfToSimple,
    BusyTimerIndicatorTransitionTypeMax,
} BusyTimerIndicatorTransitionType;

typedef struct {
    StatusLightsPreset preset;
    Color color;
} BusyStatusLightsPreset;

typedef enum {
    BusyTimerLabelTypeWork,
    BusyTimerLabelTypeRest,
    BusyTimerLabelTypeMax,
} BusyTimerLabelType;

typedef enum {
    BusyAppGlobalPresetIdBusy,
    BusyAppGlobalPresetIdCustom,
    BusyAppGlobalPresetIdMax,
} BusyAppGlobalPresetId;

typedef struct {
    const char* const header_img_path;
    const char* const start_anim_path;
    BusySettingsProfileId settings_profile_id;
} BusyAppGlobalPreset;

extern const TransitionOverlayPreset busy_transitions[BusyTransitionTypeMax];

extern const BusyStatusLightsPreset busy_status_lights[BusyStatusLightsTypeMax];

extern const TimerIndicatorPreset busy_timer_indicator_presets[BusyTimerIndicatorTypeMax];

extern const TimerIndicatorTransition
    busy_timer_indicator_transitions[BusyTimerIndicatorTransitionTypeMax];

extern const TimerLabelPreset busy_timer_label_presets[BusyTimerLabelTypeMax];

extern const BusyAppGlobalPreset busy_app_global_presets[BusyAppGlobalPresetIdMax];
