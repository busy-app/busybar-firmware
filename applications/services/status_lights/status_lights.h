/**
 * @file status_lights.h
 * @brief API for controlling Status Lights.
 */
#pragma once

#include "status_lights_common_public.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The string key for Status Lights instance access
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_STATUS_LIGHTS)`
 */
#define RECORD_STATUS_LIGHTS "status_lights"

typedef struct StatusLightsBrightness {
    uint8_t val;
} StatusLightsBrightness;

/**
 * @brief Run a preset animation with the specified color.
 *
 * @param instance status lights service instance
 * @param preset the preset animation to execute
 * @param color the color to use for the animation
 */
void status_lights_run_preset(StatusLights* instance, StatusLightsPreset preset, Color color);

/**
 * @brief Set the status lights brightness
 *
 * @param instance Pointer to the StatusLights instance
 * @param brightness Brightness value (STATUS_LIGHTS_BRIGHTNESS_MIN to STATUS_LIGHTS_BRIGHTNESS_MAX)
 */
void status_lights_set_brightness(StatusLights* instance, StatusLightsBrightness brightness);

/**
 * @brief Get the status lights brightness
 *
 * @param instance Pointer to the StatusLights instance
 * @return Brightness value (STATUS_LIGHTS_BRIGHTNESS_MIN to STATUS_LIGHTS_BRIGHTNESS_MAX)
 */
StatusLightsBrightness status_lights_get_brightness(StatusLights* instance);

#ifdef __cplusplus
}
#endif
