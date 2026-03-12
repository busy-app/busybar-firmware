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
    DeviceNameMessageTypeGet,
    DeviceNameMessageTypeSet,
    DeviceNameMessageTypeMqttUpdate,
    DeviceNameMessageTypeMax,
} DeviceNameMessageType;

typedef struct {
    FuriString* name;
} DeviceNameMessageGet;

typedef struct {
    FuriString* name;
    FuriString* error;
    bool* result;
} DeviceNameMessageSet;

typedef union {
    DeviceNameMessageGet get;
    DeviceNameMessageSet set;
} DeviceNameMessageData;

typedef struct {
    FuriApiLock api_lock;
    DeviceNameMessageType type;
    DeviceNameMessageData data;
} DeviceNameMessage;

#ifdef __cplusplus
}
#endif
