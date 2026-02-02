/**
 * @file status_lights_backend.h
 * @brief API for controlling Status Lights from f64 side.
 */
#pragma once

#include "status_lights_common_public.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The string key for Status Lights instance access from f64 side
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_STATUS_LIGHTS)`
 */
#define RECORD_STATUS_LIGHTS "status_lights"

/**
 * @brief Run a preset animation with the specified color.
 *
 * @param instance status lights service instance
 * @param preset the preset animation to execute
 * @param color the color to use for the animation
 */
void status_lights_run_preset(StatusLights* instance, StatusLightsPreset preset, Color color);

#ifdef __cplusplus
}
#endif
