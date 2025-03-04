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
 * @brief The string key for Status Lights instance access
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_STATUS_LIGHTS)`
 */
#define RECORD_STATUS_LIGHTS "status_lights"

/**
 * @brief Send command to Status Lights
 *
 * @param instance The Status Lights instance
 * @param command The StatusLightsCommand to send
 */
void status_lights_send_command(StatusLights* instance, StatusLightsCommand command);

#ifdef __cplusplus
}
#endif
