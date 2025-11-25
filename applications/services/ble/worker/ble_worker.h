#pragma once
#include "../service/ble_service.h"

#define BLE_REMOTE_ADDRESS_SIZE (6)

typedef void (*BleConnectionStateChanged)(
    void* context,
    bool conntected,
    const uint8_t remote_dev_address[BLE_REMOTE_ADDRESS_SIZE]);

void ble_worker_init(BleConnectionStateChanged connect_callback, void* ctx);

bool ble_worker_register_service(BleServiceObject* service);

void ble_worker_send(uint16_t handle, uint16_t data_size, const uint8_t* data, uint16_t cccd_value);

void ble_worker_receive_confirm(uint16_t handle, uint8_t cccd_value);

void ble_worker_start();

void ble_worker_stop();

bool ble_worker_forget_pairing();

bool ble_worker_pairing_exists();

void ble_worker_test_after_init();

void ble_worker_set_name(const char* new_name);
