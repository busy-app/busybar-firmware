#pragma once

#include <furi.h>

#define BLE_WORKER_ATTR_HEADER_SIZE 3

bool ble_worker_event_handler_indicate_confirm(size_t data_size, void* data, void* context);
bool ble_worker_event_handler_mtu(size_t data_size, void* data, void* context);
bool ble_worker_event_handler_write_event(size_t data_size, void* data, void* context);
