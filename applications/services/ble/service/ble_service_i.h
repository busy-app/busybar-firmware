#pragma once

#include "ble_service.h"
#include "ble_characteristic.h"

#include <furi.h>

#define BLE_ATT_PROPERTY_READ     0x02
#define BLE_ATT_PROPERTY_WRITE    0x08
#define BLE_ATT_PROPERTY_NOTIFY   0x10
#define BLE_ATT_PROPERTY_INDICATE 0x20

struct BleServiceObject {
    BleServiceState state;
    const BleServiceDescriptor* config;
    BleCharacteristicObject** chars;

    FuriMessageQueue* message_queue;
    FuriMutex* service_lock;
    IntercomChannel* intercom_ch;

    FuriSemaphore* frame_lock;
    size_t frame_size;
    uint8_t* frame_buf;

    BleServiceStateChangeCallback state_change_callback;
    BleServiceStateChangeCallbackContext* state_callback_context;

    void* context;
#if defined(SI917)
    void* service_handler;
    uint16_t handle;
#endif
};

void ble_service_enqueue_message(BleServiceObject* instance, BleCommand command, uint8_t ch_index);
void ble_service_enqueue_run(BleServiceObject* instance);

void ble_service_prepare_send_intercom_frame(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    BleCommand command,
    size_t data_size,
    void* data);

void ble_service_switch_state(BleServiceObject* instance, BleServiceState new_state);

bool ble_service_lock(BleServiceObject* instance);
void ble_service_unlock(BleServiceObject* instance);
void ble_service_send_intercom_frame(BleServiceObject* instance);
