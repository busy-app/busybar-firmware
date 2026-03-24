/**
 * @brief Device Name service internal header
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "device_name.h"

#include <furi.h>
#include <mqtt/mqtt.h>
#include <toolbox/api_lock.h>

#include "settings/device_name_settings.h"

#define TAG "DeviceName"

struct DeviceName {
    FuriEventLoop* event_loop;
    FuriMessageQueue* queue;
    FuriPubSub* pubsub;
    DeviceNameSettings settings;
    Mqtt* mqtt;
    FuriPubSub* mqtt_events_pubsub;
};

typedef enum {
    DeviceNameMessageTypeGetName,
    DeviceNameMessageTypeSetName,
    DeviceNameMessageTypeMqttPublish,
    DeviceNameMessageTypeMax,
} DeviceNameMessageType;

typedef struct {
    FuriString* name;
} DeviceNameMessageGetName;

typedef struct {
    const FuriString* name;
    DeviceNameError* error;
} DeviceNameMessageSetName;

typedef union {
    DeviceNameMessageGetName get_name;
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
