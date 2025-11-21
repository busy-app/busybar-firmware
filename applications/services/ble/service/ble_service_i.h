#pragma once

#include "ble_service.h"
#include "ble_characteristic.h"
#include "ble_service_command.h"

#include <furi.h>

#define BLE_ATT_PROPERTY_READ     0x02
#define BLE_ATT_PROPERTY_WRITE    0x08
#define BLE_ATT_PROPERTY_NOTIFY   0x10
#define BLE_ATT_PROPERTY_INDICATE 0x20

struct BleServiceObject {
    bool ready;
    const BleServiceDescriptor* config;
    BleCharacteristicObject** chars;

    FuriMessageQueue* message_queue;
    FuriMutex* service_lock;
    Intercom* intercom;

    FuriSemaphore* frame_lock;
    bool frame_pending;
    size_t buffer_size;
    uint8_t* frame_buf;

    BleServiceStateChangeCallback state_change_callback;
    BleServiceStateChangeCallbackContext* state_callback_context;

    void* context;
#if defined(SI917)
    void* service_handler;
    uint16_t handle;
#endif
};

///TODO: make this function to return result value
typedef void (*BleParseIntercomServiceDataCharacteristicExtraAction)(
    BleCharacteristicObject* characteristic);

void ble_service_enqueue_message(BleServiceObject* instance);
void ble_service_enqueue_run(BleServiceObject* instance);

void ble_service_prepare_send_intercom_frame(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    BleServiceCommandEnum command,
    size_t data_size,
    void* data);

void ble_service_switch_state(BleServiceObject* instance, BleServiceState new_state);

bool ble_service_lock(BleServiceObject* instance);
void ble_service_unlock(BleServiceObject* instance);
void ble_service_send_intercom_frame(BleServiceObject* instance);

size_t ble_service_count_characteristics_and_size(
    BleServiceObject* instance,
    bool modified_only,
    BleCharacteristicCountType* characteristics_count);

BleIntercomServiceData* ble_service_create_intercom_service_data_pack(
    BleServiceObject* instance,
    bool modified_only,
    size_t* output_pack_size);

bool ble_service_parse_intercom_service_data(
    BleServiceObject* instance,
    const BleIntercomServiceData* data,
    BleParseIntercomServiceDataCharacteristicExtraAction action);
