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

/** Status lights preset pattern */
typedef enum {
    StatusLightsPresetOff, /**< Status lights off */
    StatusLightsPresetStaticColor, /**< Static color */
    StatusLightsPresetFade, /**< White fade pattern */
    StatusLightsPresetRainbowGradient, /**< Rainbow gradient pattern */

    StatusLightsPresetMax, /**< Number of presets */
} StatusLightsPreset;

/** Status lights command */
typedef struct {
    StatusLightsPreset preset; /**< Preset pattern */
    Color color; /**< Color value */
} StatusLightsCommand;

#ifdef __cplusplus
}
#endif
