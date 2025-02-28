#pragma once

#include <furi.h>
#include <gui_lvgl/gui_lvgl.h>

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
    GuiLvgl* gui;

    FuriPubSub* light_sensor_events;
    FuriPubSubSubscription* light_sensor_subscription;

    lv_obj_t* label_light_raw;
    lv_obj_t* label_lux_instant;
    lv_obj_t* label_lux_mean;
    lv_obj_t* label_light_level;

    uint16_t raw_600nm;
    uint16_t raw_840nm;
    float lux_instant;
    float lux_mean;
    uint8_t light_level;
} LightSensorTestApp;
