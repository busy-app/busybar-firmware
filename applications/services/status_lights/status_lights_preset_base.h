#pragma once

#include "status_lights.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void StatusLightsGenericPreset;

typedef StatusLightsGenericPreset* (*StatusLightsPresetAlloc)(void);
typedef void (*StatusLightsPresetFree)(StatusLightsGenericPreset* instance);
typedef void (*StatusLightsPresetRun)(StatusLightsGenericPreset* instance, Color* color);

typedef struct {
    int period_ms;
    StatusLightsPresetAlloc alloc;
    StatusLightsPresetFree free;
    StatusLightsPresetRun run;
} StatusLightsPresetBase;

#ifdef __cplusplus
}
#endif
