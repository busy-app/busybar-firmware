#pragma once

#include <toolbox/color.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque StatusLights type declaration. */
typedef struct StatusLights StatusLights;

typedef enum {
    StatusLightsCommandSetManual,
    StatusLightsCommandSetPreset,
} StatusLightsCommandType;

typedef enum {
    StatusLightsPresetRainbowGradient,
    StatusLightsPresetWhiteFade,

    StatusLightsPresetNum,
} StatusLightsPreset;

typedef struct {
    StatusLightsCommandType type;
    union {
        Color color;
        StatusLightsPreset preset;
    };
} StatusLightsCommand;

#ifdef __cplusplus
}
#endif
