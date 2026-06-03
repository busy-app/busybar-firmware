#pragma once

#include "ble_security.h"
#include "ble_advertise.h"

#include <furi.h>

typedef enum {
    BleDeviceStateIdle,
    BleDeviceStateAdvertising,
    BleDeviceStateConnected,
    BleDeviceStateError,
} BleDeviceState;

typedef struct BleDevice BleDevice;

BleDevice* ble_device_alloc(/*BleDeviceType and possibly address*/);
void ble_device_free(BleDevice* instance);

//Service register handlers
void ble_device_register_service();

//CONNECTION handlers
//----------------------------------------------------------------------------
//Create BleDevice* remote instance inside of BsbDevice and stores parameters
bool ble_device_connect(BleDevice* instance, const uint8_t* const peer_address);

//Destroys BleDevice* remote instance inside of BsbDevice.
bool ble_device_disconnect(BleDevice* instance);

bool ble_device_is_connected(BleDevice* instance);

void ble_device_set_name(BleDevice* instance, const char* name);

bool ble_device_start_advertise(BleDevice* instance);

bool ble_device_stop_advertise(BleDevice* instance);

BleAdvertiseContext* ble_device_get_advertise_context(BleDevice* instance);
// const BleAdvertiseContext*
// //A
// void ble_device_update_connection_begin();

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
void ble_device_forget_paired(/*address*/);

//Forgets all paired devices
// void ble_device_forget_all();
