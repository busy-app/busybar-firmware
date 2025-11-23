#pragma once

#include "busy.h"
#include <toolbox/vector3.h>

#define BUSY_LOTTIE_SLOT_TEMPLATE \
    "{"                           \
    " \"wave_offset\": {"         \
    "  \"p\": {"                  \
    "   \"a\": 0,"                \
    "   \"k\": ["                 \
    "     %f,"                    \
    "     %f,"                    \
    "     %f"                     \
    "   ]"                        \
    "}}}"

typedef struct {
    StatusLightsPreset preset;
    Color color;
} BusyStatusLightsPreset;

typedef struct {
    const Vector3 position_start;
    const Vector3 position_end;
    const char* anim_path;
    const char* lottie_path;
    const char* image_path;
    Color countdown_main_color;
    Color countdown_blink_color;
} BusySceneTimerIntervalAsset;

typedef enum {
    BusySceneTimerIntervalAssetIdBusy,
    BusySceneTimerIntervalAssetIdRest,
    BusySceneTimerIntervalAssetIdMax,
} BusySceneTimerIntervalAssetId;

extern const TransitionOverlayPreset busy_transitions[BusyTransitionTypeMax];

extern const BusyStatusLightsPreset busy_status_lights[BusyStatusLightsTypeMax];

extern const BusySceneTimerIntervalAsset
    busy_scene_timer_interval_assets[BusySceneTimerIntervalAssetIdMax];
