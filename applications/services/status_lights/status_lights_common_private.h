/**
 * @file status_lights_private.h
 * @brief Common private definitions for Status Lights.
 */
#pragma once

#include "status_lights_common_public.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    StatusLightsCommandIdRunPreset,
    StatusLightsCommandIdSetBrightness,

    StatusLightsCommandIdsCount
} StatusLightsCommandId;

/** Status lights command */
typedef struct {
    union {
        struct {
            StatusLightsPreset preset; /**< Preset pattern */
            Color color; /**< Color value */
        } as_run_preset;

        struct {
            float brightness; /**< Brightness value */
        } as_set_brightness;
    };

    StatusLightsCommandId id;
} StatusLightsCommand;

#ifdef __cplusplus
}
#endif
