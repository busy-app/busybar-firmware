#pragma once

#include "ble_service_config_types.h"

#include <furi.h>

typedef struct BleServiceObject BleServiceObject;

typedef void (
    *BleServiceStateChangeCallback)(BleServiceObject* instance, BleServiceState new_state);

BleServiceObject* ble_service_alloc(
    const BleServiceDescriptor* service_config,
    FuriMessageQueue* dest_queue,
    IntercomChannel* intercom_ch);

bool ble_service_process(BleServiceObject* instance, const BleServiceCommand* msg);
void ble_service_process_mailbox(
    BleServiceObject* instance,
    const BleIntercomFrameGeneric* input_frame);
void ble_service_enqueue_init(BleServiceObject* instance);
