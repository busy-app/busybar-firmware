#pragma once
#include <furi/furi.h>
#include <light_sensor/light_sensor_common.h>
#define RECORD_LIGHT_SENSOR "light_sensor"
typedef struct LightSensor LightSensor;
FuriState* light_sensor_get_state(LightSensor*);
