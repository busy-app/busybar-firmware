/**
 * @file status_lights_i.h
 * @brief Common private definitions for Status Lights.
 */
#pragma once

#include "status_lights_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STATUS_LIGHTS_BRIGHTNESS_MIN     (0)
#define STATUS_LIGHTS_BRIGHTNESS_MAX     (100)
#define STATUS_LIGHTS_BRIGHTNESS_DEFAULT (STATUS_LIGHTS_BRIGHTNESS_MAX)

typedef enum {
    StatusLightsCommandIdRunPreset,
    StatusLightsCommandIdSetBrightness,
    StatusLightsCommandIdMax,
} StatusLightsCommandId;

typedef struct {
    StatusLightsPreset preset; /**< Preset pattern */
    Color color; /**< Color value */
} StatusLightsCommandRunPreset;

typedef struct {
    uint8_t brightness; /**< Brightness value */
} StatusLightsCommandSetBrightness;

/** Status lights command */
typedef struct {
    StatusLightsCommandId id;
    union {
        StatusLightsCommandRunPreset run_preset;
        StatusLightsCommandSetBrightness set_brightness;
    };
} StatusLightsCommand;

#ifdef __cplusplus
}
#endif
