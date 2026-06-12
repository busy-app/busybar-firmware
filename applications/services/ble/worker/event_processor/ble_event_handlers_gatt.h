#pragma once

#include <furi.h>

bool ble_event_handler_gatt_mtu(size_t data_size, void* data, void* context);
bool ble_event_handler_gatt_write_event(size_t data_size, void* data, void* context);
bool ble_event_handler_gatt_read_request_event(size_t data_size, void* data, void* context);
