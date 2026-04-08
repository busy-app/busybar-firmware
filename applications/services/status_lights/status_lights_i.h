#pragma once

#include "status_lights.h"
#include "status_lights_common_i.h"

#include <intercom/intercom.h>

#include <api_lock.h>

#define TAG "StatusLights"

struct StatusLights {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    Intercom* intercom;
    IntercomChannel* intercom_ch;
    StatusLightsBrightness brightness;
};

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

void status_lights_api_unlock(StatusLightsApiMessage* api_message, StatusLightsStatus status);
