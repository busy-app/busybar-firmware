/**
 * @file status_lights.h
 * @brief API for controlling Status Lights.
 */
#pragma once

#include "status_lights_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The string key for Status Lights instance access.
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_STATUS_LIGHTS)`
 */
#define RECORD_STATUS_LIGHTS "status_lights"

/**
 * @brief Enumeration of possible StatusLights statuses.
 */
typedef enum {
    StatusLightsStatusOk, /**< Request executed successfully */
    StatusLightsStatusTimeout, /**< Request timed out (e.g. backend not responding) */
    StatusLightsStatusError, /**< Request failed due to internal error */
} StatusLightsStatus;

typedef struct StatusLightsBrightness {
    uint8_t val;
} StatusLightsBrightness;

/**
 * @brief Run a preset animation with the specified color.
 *
 * @param instance status lights service instance
 * @param preset the preset animation to execute
 * @param color the color to use for the animation
 * @return StatusLightsStatusOk on success, any other enum value on error
 */
StatusLightsStatus
    status_lights_run_preset(StatusLights* instance, StatusLightsPreset preset, Color color);

/**
 * @brief Set the status lights brightness
 *
 * @param instance Pointer to the StatusLights instance
 * @param brightness Brightness value (STATUS_LIGHTS_BRIGHTNESS_MIN to STATUS_LIGHTS_BRIGHTNESS_MAX)
 * @return StatusLightsStatusOk on success, any other enum value on error
 */
StatusLightsStatus
    status_lights_set_brightness(StatusLights* instance, StatusLightsBrightness brightness);

/**
 * @brief Get the status lights brightness
 *
 * @param instance Pointer to the StatusLights instance
 * @param brightness Pointer to the variable to contain the brightness value (must be allocated)
 * @return StatusLightsStatusOk on success, any other enum value on error
 */
StatusLightsStatus
    status_lights_get_brightness(StatusLights* instance, StatusLightsBrightness* brightness);

#ifdef __cplusplus
}
#endif
