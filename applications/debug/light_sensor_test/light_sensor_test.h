#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/modules/flex_layout.h>
#include <gui/modules/label.h>

#include <light_sensor/light_sensor.h>
#include <light_sensor/light_sensor_common.h>

typedef enum {
    LightSensorTestAppEventExit,
    LightSensorTestAppEventLightLevelUpdate,
} LightSensorTestAppEventType;

typedef struct {
    LightSensorTestAppEventType type;
    union {
        LightSensorState lighth_sensor_state;
    };
} LightSensorTestAppEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriEventLoopTimer* timer;
    Gui* gui;

    LightSensor* light_sensor;
    FuriStateSub* light_sensor_events;
    LightSensorState light_sensor_state;

    FlexLayout* flex;
    Label* label_light_raw_600nm;
    Label* label_light_raw_840nm;
    Label* label_lux_instant;
    Label* label_lux_mean;
    Label* label_light_level;

    uint16_t raw_600nm;
    uint16_t raw_840nm;
} LightSensorTestApp;
