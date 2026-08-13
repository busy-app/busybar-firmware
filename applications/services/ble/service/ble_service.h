#pragma once

#include "../ble_log.h"
#include "../ble_intercom_types.h"
#include "ble_service_config_types.h"

#include <furi.h>

typedef struct BleServiceObject BleServiceObject;

BleServiceObject* ble_service_alloc(
    const BleServiceDescriptor* service_config,
    FuriMessageQueue* message_queue,
    IntercomChannel* intercom_ch);

bool ble_service_process(BleServiceObject* instance);
void ble_service_process_mailbox(
    BleServiceObject* instance,
    const BleIntercomFrameGeneric* input_frame);

bool ble_service_is_ready(BleServiceObject* instance);

const char* ble_service_get_name(BleServiceObject* instance);

void ble_service_get_error(BleServiceObject* instance, FuriString* error);

void ble_service_enqueue_init(BleServiceObject* instance);
void ble_service_enqueue_run(BleServiceObject* instance);
void ble_service_deinit(BleServiceObject* instance);

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
