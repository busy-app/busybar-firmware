#pragma once

#include "status_lights.h"
#include "status_lights_common_private.h"

#include <intercom/intercom.h>

#include <api_lock.h>

#define STATUS_LIGHTS_BRIGHTNESS_MIN     (0)
#define STATUS_LIGHTS_BRIGHTNESS_MAX     (100)
#define STATUS_LIGHTS_BRIGHTNESS_DEFAULT (50)

#define TAG "StatusLights"

struct StatusLights {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    Intercom* intercom;
    IntercomChannel* intercom_ch;
    StatusLightsBrightness brightness;
};

typedef enum {
    StatusLightsApiMessageTypeInit,
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

void status_lights_init(StatusLights* instance);

void status_lights_api_unlock(StatusLightsApiMessage* api_message, StatusLightsStatus status);
