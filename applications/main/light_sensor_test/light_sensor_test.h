#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/label.h>

#include <light_sensor/light_sensor.h>

typedef enum {
    LightSensorTestAppEventExit,
    LightSensorTestAppEventLightLevelUpdate,
} LightSensorTestAppEventType;

typedef struct {
    LightSensorTestAppEventType type;
    union {
        uint8_t light_level;
    };
} LightSensorTestAppEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriEventLoopTimer* timer;
    Gui* gui;
    GuiInputSubscription* input_events;

    FuriPubSub* light_sensor_events;
    FuriPubSubSubscription* light_sensor_subscription;

    Widget* app_window;
    Label* label_light_raw;
    Label* label_lux_instant;
    Label* label_lux_mean;
    Label* label_light_level;

    uint16_t raw_600nm;
    uint16_t raw_840nm;
    float lux_instant;
    float lux_mean;
    uint8_t light_level;
} LightSensorTestApp;
