#pragma once

#include "status_lights.h"
#include "status_lights_common_private.h"

#include <intercom/intercom.h>

#include <api_lock.h>

#define STATUS_LIGHTS_BRIGHTNESS_MIN     (0)
#define STATUS_LIGHTS_BRIGHTNESS_MAX     (100)
#define STATUS_LIGHTS_BRIGHTNESS_DEFAULT (50)

struct StatusLights {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    IntercomChannel* intercom_ch;

    StatusLightsBrightness brightness;
};

typedef enum {
    StatusLightsMessageTypeSetBrightness,
    StatusLightsMessageTypeGetBrightness,
    StatusLightsMessageTypeSetRunPreset,

    StatusLightsMessageTypesCount,
} StatusLightsMessageType;

typedef struct {
    FuriApiLock api_lock;
    StatusLightsMessageType type;
    union {
        struct {
            StatusLightsBrightness brightness;
        } as_set_brightness;

        struct {
            StatusLightsBrightness* brightness;
        } as_get_brightness;

        struct {
            StatusLightsPreset preset;
            Color color;
        } as_run_preset;
    };
} StatusLightsMessage;
