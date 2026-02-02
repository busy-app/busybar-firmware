#pragma once

#include "ble_service_device_events.h"
#include "../ble_service_i.h"

typedef uint32_t BleServiceDeviceEvents;

bool ble_service_device_events_init(void* object);
bool ble_service_device_events_run(void* object);
