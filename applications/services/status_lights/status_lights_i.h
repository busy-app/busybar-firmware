#pragma once

#include "status_lights.h"
#include "status_lights_common_i.h"

#include <intercom/intercom.h>

#include <api_lock.h>

#define TAG "StatusLights"

typedef enum {
    StatusLightsCustomEventInit = 1UL << 0,
    StatusLightsCustomEventDeinit = 1UL << 1,
    StatusLightsCustomEventRequest = 1UL << 2,
} StatusLightsCustomEvent;

typedef enum {
    StatusLightsApiMessageTypeSetBrightness,
    StatusLightsApiMessageTypeGetBrightness,
    StatusLightsApiMessageTypeRunPreset,
    StatusLightsApiMessageTypeMax,
} StatusLightsApiMessageType;

typedef struct {
    StatusLightsBrightness brightness;
} StatusLightsApiMessageSetBrightness;

typedef struct {
    StatusLightsBrightness* brightness;
} StatusLightsApiMessageGetBrightness;

typedef struct {
    StatusLightsPreset preset;
    Color color;
} StatusLightsApiMessageRunPreset;

typedef struct {
    StatusLightsApiMessageType type;
    StatusLightsStatus* status;
    FuriApiLock lock;
    union {
        StatusLightsApiMessageSetBrightness set_brightness;
        StatusLightsApiMessageGetBrightness get_brightness;
        StatusLightsApiMessageRunPreset run_preset;
    };
} StatusLightsApiMessage;

struct StatusLights {
    FuriEventLoop* event_loop;
    FuriSemaphore* api_semaphore;
    Intercom* intercom;
    IntercomChannel* intercom_ch;
    StatusLightsApiMessage api_message;
    StatusLightsBrightness brightness;
};

bool status_lights_api_is_locked(StatusLights* instance);

void status_lights_api_unlock(StatusLights* instance, StatusLightsStatus status);
