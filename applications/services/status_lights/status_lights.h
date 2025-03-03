/**
 * @file status_lights.h
 * @brief API for controlling Status Lights.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The string key for Status Lights instance access
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_STATUS_LIGHTS)`
 */
#define RECORD_STATUS_LIGHTS "status_lights"

/** Opaque StatusLights type declaration. */
typedef struct StatusLights StatusLights;

typedef enum {
    StatusLightsPatternTypeManual,
    StatusLightsPatternTypePreset,
} StatusLightsPatternType;

#ifdef __cplusplus
}
#endif
