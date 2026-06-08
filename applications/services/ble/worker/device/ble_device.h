#pragma once

#include "ble_connection.h"
#include "ble_security.h"
#include "ble_advertise.h"
#include "../transmitter/ble_transmitter.h"

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
void ble_device_register_service();

//CONNECTION handlers
//----------------------------------------------------------------------------
//Create BleDevice* remote instance inside of BsbDevice and stores parameters
BleConnectionContext* ble_device_get_connection_context(BleDevice* instance);

bool ble_device_connection_open(
    BleDevice* instance,
    BleDeviceAddressType type,
    const uint8_t* peer_addr);

bool ble_device_connection_close(BleDevice* instance);

//Destroys BleDevice* remote instance inside of BsbDevice.
bool ble_device_disconnect(BleDevice* instance);

bool ble_device_is_connected(BleDevice* instance);

void ble_device_set_name(BleDevice* instance, const char* name);

bool ble_device_start(BleDevice* instance);

bool ble_device_stop(BleDevice* instance);

BleAdvertiseContext* ble_device_get_advertise_context(BleDevice* instance);

// //Stores new connection parameters if update was successful
// void ble_device_update_connection_end();

// void ble_device_update_mtu();

// void ble_device_update_remote_features(/*Address*/);
//----------------------------------------------------------------------------
void ble_device_process_read_request();

void ble_device_receive_confirm();
//----------------------------------------------------------------------------
//PAIRING SMP HANDLERS

BleSecurityData* ble_device_get_security_data(BleDevice* instance);
bool ble_device_is_paired(BleDevice* instance);

//Starts pairing process on smp request
void ble_device_pairing_begin(/*Pairing data*/);

//Finish pairing on encryption started
void ble_device_pairing_end();

//Forgets paired device
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
