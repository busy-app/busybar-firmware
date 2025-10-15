#pragma once

#include "ble_service_config_types.h"

#include <furi.h>

typedef struct BleServiceObject BleServiceObject;

typedef void BleServiceStateChangeCallbackContext;

typedef void (*BleServiceStateChangeCallback)(BleServiceStateChangeCallbackContext* ctx);

BleServiceObject* ble_service_alloc(
    const BleServiceDescriptor* service_config,
    FuriMessageQueue* message_queue,
    IntercomChannel* intercom_ch,
    BleServiceStateChangeCallback state_callback,
    BleServiceStateChangeCallbackContext* ctx);

bool ble_service_process(BleServiceObject* instance, const BleServiceCommand* msg);
void ble_service_process_mailbox(
    BleServiceObject* instance,
    const BleIntercomFrameGeneric* input_frame);

BleServiceState ble_service_get_state(BleServiceObject* instance);

void ble_service_enqueue_init(BleServiceObject* instance);

void ble_service_write_data(
    BleServiceObject* instance,
    uint8_t index,
    const void* data,
    const size_t data_size);

void ble_service_register_update_callback(
    BleServiceObject* instance,
    uint16_t index,
    BleDataUpdatedCallback cb,
    void* ctx);

void ble_service_register_transmission_done_callback(
    BleServiceObject* instance,
    uint16_t index,
    BleDataTransmitDoneCallback cb,
    void* ctx);
