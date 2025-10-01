#pragma once

#include <furi.h>

#define RECORD_DEVICE_NAME "device_name"

typedef struct DeviceName DeviceName;

void device_name_get(DeviceName* instance, FuriString* name);
void device_name_set(DeviceName* instance, FuriString* name);

#define DEVICE_NAME_GET(name)                                        \
    do {                                                             \
        DeviceName* dev_name = furi_record_open(RECORD_DEVICE_NAME); \
        device_name_get(dev_name, name);                             \
        furi_record_close(RECORD_DEVICE_NAME);                       \
    } while(false);

#define DEVICE_NAME_SET(name)                                        \
    do {                                                             \
        DeviceName* dev_name = furi_record_open(RECORD_DEVICE_NAME); \
        device_name_set(dev_name, name);                             \
        furi_record_close(RECORD_DEVICE_NAME);                       \
    } while(false);
