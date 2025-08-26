#pragma once
#include "../service/ble_service.h"

void ble_worker_init();

// void ble_worker_init_service(BleIntercomFrameServiceConfig* config);

bool ble_worker_register_service(BleServiceObject* service);

void ble_worker_set_value(
    uint16_t service_index,
    uint16_t char_index,
    uint16_t data_size,
    const uint8_t* data);

void ble_worker_set_data(uint16_t handle, uint16_t data_size, const uint8_t* data);
void ble_worker_notify(uint16_t handle, uint16_t data_size, const uint8_t* data);

void ble_worker_start();

void ble_worker_stop();

void ble_worker_test_after_init();
