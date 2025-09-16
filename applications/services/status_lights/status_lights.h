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

#define STATUS_LIGHTS_BRIGHTNESS_MIN  (0)
#define STATUS_LIGHTS_BRIGHTNESS_MAX  (100)
#define STATUS_LIGHTS_BRIGHTNESS_AUTO (255)

void status_lights_run_preset(StatusLights* instance, StatusLightsPreset preset, Color color);

void status_lights_set_brightness(StatusLights* instance, uint8_t brightness);
uint8_t status_lights_get_brightness(StatusLights* instance);

#ifdef __cplusplus
}
#endif
