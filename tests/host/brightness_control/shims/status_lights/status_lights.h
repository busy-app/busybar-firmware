#pragma once
#include <stdint.h>
#define RECORD_STATUS_LIGHTS "status_lights"
typedef struct StatusLights StatusLights;
typedef struct {
    uint8_t val;
} StatusLightsBrightness;
typedef enum {
    StatusLightsStatusOk,
    StatusLightsStatusTimeout,
    StatusLightsStatusError,
} StatusLightsStatus;
StatusLightsStatus status_lights_set_brightness(StatusLights*, StatusLightsBrightness);
