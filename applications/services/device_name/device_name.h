#pragma once

#include <furi.h>

#define RECORD_DEVICE_NAME "device_name"

typedef struct DeviceName DeviceName;

void device_name_get(DeviceName* instance, FuriString* name);
bool device_name_set(DeviceName* instance, FuriString* name);
