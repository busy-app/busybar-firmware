#pragma once

#include "ble_connection.h"
#include "ble_security.h"
#include "../transport/ble_transmitter.h"
#include "../../service/ble_service.h"

#include <furi.h>

typedef enum {
    BleDeviceStateIdle,
    BleDeviceStateAdvertising,
    BleDeviceStateConnected,
    BleDeviceStateError,
} BleDeviceState;

typedef struct BleDevice BleDevice;

BleDevice* ble_device_alloc(BleTransmitter* transmitter);
void ble_device_free(BleDevice* instance);

//Service register handlers
bool ble_device_register_service(BleDevice* instance, BleServiceObject* service);

//CONNECTION handlers
BleConnectionContext* ble_device_get_connection_context(BleDevice* instance);

bool ble_device_connection_open(
    BleDevice* instance,
    BleDeviceAddressType type,
    const uint8_t* peer_addr);

bool ble_device_connection_close(BleDevice* instance);

bool ble_device_disconnect(BleDevice* instance);

bool ble_device_is_connected(BleDevice* instance);

void ble_device_set_name(BleDevice* instance, const char* name);

bool ble_device_start(BleDevice* instance);

bool ble_device_stop(BleDevice* instance);

void ble_device_set_mtu(BleDevice* instance, uint16_t mtu);

//----------------------------------------------------------------------------
bool ble_device_process_write_request(
    BleDevice* instance,
    const uint8_t* remote_addr,
    const uint16_t handle,
    const size_t data_size,
    const void* data);

bool ble_device_process_read_request(
    BleDevice* instance,
    uint8_t* addr,
    uint8_t type,
    uint16_t handle,
    uint16_t offset);

void ble_device_receive_confirm(BleDevice* instance, uint16_t handle, uint8_t cccd_value);
//----------------------------------------------------------------------------
//PAIRING SMP HANDLERS
BleSecurityData* ble_device_get_security_data(BleDevice* instance);
bool ble_device_is_paired(BleDevice* instance);

void ble_device_response_pairing_capabilities(BleDevice* instance);

void ble_device_request_pairing(BleDevice* instance);

bool ble_device_send_encryption_response(BleDevice* instance);

void ble_device_handle_encryption_start(
    BleDevice* instance,
    rsi_bt_event_encryption_enabled_t* encryption_data);

bool ble_device_forget_paired(BleDevice* instance);

//Forgets all paired devices
// void ble_device_forget_all();

//----------------------------------------------------------------------------
void ble_device_send_data(
    BleDevice* instance,
    uint16_t handle,
    uint16_t data_size,
    const uint8_t* data,
    uint16_t cccd_value);
