/**
 * @file status_lights_common_public.h
 * @brief Common public definitions for Status Lights.
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
    StatusLightsPresetBlink, /**< Blink pattern */

    StatusLightsPresetsCount, /**< Number of presets */
} StatusLightsPreset;

#ifdef __cplusplus
}
#endif
