#pragma once

#include "generic_access.h"
#include "../ble_service_i.h"

#include <furi.h>

#define TAG "BleGAP"

typedef enum {
    BleGenericAccessCharacterDeviceName,
    BleGenericAccessCharacterAppearance,
} BleSrvGenericAccess;

bool ble_service_generic_access_init(void* object);
bool ble_service_generic_access_run(void* object, size_t data_size, const void* data);
