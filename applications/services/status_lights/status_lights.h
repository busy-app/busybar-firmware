/**
 * @file status_lights.h
 * @brief API for controlling Status Lights.
 */
#pragma once

#include <stdint.h>

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
    StatusLightsCommandSetManual,
    StatusLightsCommandSetPreset,
} StatusLightsCommandType;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} StatusLightsColor;

typedef enum {
    StatusLightsPresetRainbowGradient,
    StatusLightsPresetWhiteFade,

    StatusLightsPresetNum,
} StatusLightsPreset;

typedef struct {
    StatusLightsCommandType type;
    union {
        StatusLightsColor manual;
        StatusLightsPreset preset;
    };
} StatusLightsCommand;

void status_light_send_command(StatusLights* instance, StatusLightsCommand command);

#ifdef __cplusplus
}
#endif
