#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "device_name.h"

#include <furi.h>
#include <mqtt/mqtt.h>
#include <toolbox/api_lock.h>

#define TAG "DeviceName"

struct DeviceName {
    FuriEventLoop* event_loop;
    FuriMessageQueue* queue;
    FuriState* state;
    Mqtt* mqtt;
};

typedef enum {
    DeviceNameMessageTypeSetName,
    DeviceNameMessageTypeMqttPublish,
    DeviceNameMessageTypeMax,
} DeviceNameMessageType;

typedef struct {
    const char* name;
    DeviceNameError* error;
} DeviceNameMessageSetName;

typedef union {
    DeviceNameMessageSetName set_name;
} DeviceNameMessageData;

typedef struct {
    FuriApiLock api_lock;
    DeviceNameMessageType type;
    DeviceNameMessageData data;
} DeviceNameMessage;

DeviceNameError device_name_validate(const char* name);

#ifdef __cplusplus
}
#endif
