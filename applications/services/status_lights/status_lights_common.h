/**
 * @file status_lights_common.h
 * @brief Common definitions for Status Lights.
 */
#pragma once

#include <toolbox/color.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque StatusLights type declaration. */
typedef struct StatusLights StatusLights;

/** Status lights command type */
typedef enum {
    StatusLightsCommandSetManual, /**< Command to set manual color */
    StatusLightsCommandSetPreset, /**< Command to set preset pattern */
} StatusLightsCommandType;

/** Status lights preset pattern */
typedef enum {
    StatusLightsPresetRainbowGradient, /**< Rainbow gradient pattern */
    StatusLightsPresetWhiteFade, /**< White fade pattern */

    StatusLightsPresetNum, /**< Number of presets */
} StatusLightsPreset;

typedef struct {
    StatusLightsCommandType type; /**< Command type */
    union {
        Color color; /**< Manual color settings */
        StatusLightsPreset preset; /**< Preset pattern */
    };
} StatusLightsCommand;

#ifdef __cplusplus
}
#endif
