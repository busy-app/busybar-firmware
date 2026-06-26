#pragma once

#include "status_lights_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define REPEAT_INFINITE (0)

typedef void StatusLightsGenericPreset;

typedef StatusLightsGenericPreset* (*StatusLightsPresetAlloc)(const Color* color);
typedef void (*StatusLightsPresetFree)(StatusLightsGenericPreset* instance);
typedef void (*StatusLightsPresetRun)(StatusLightsGenericPreset* instance, Color* color);

typedef struct {
    StatusLightsPresetAlloc alloc;
    StatusLightsPresetFree free;
    StatusLightsPresetRun run;
    uint32_t period_ms;
    uint32_t repeat_count;
    bool override_brightness;
} StatusLightsPresetBase;

#ifdef __cplusplus
}
#endif
