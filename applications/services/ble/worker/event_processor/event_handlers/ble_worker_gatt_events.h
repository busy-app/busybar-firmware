#pragma once

#include <furi.h>

bool ble_worker_event_handler_mtu(size_t data_size, void* data, void* context);
bool ble_worker_event_handler_write_event(size_t data_size, void* data, void* context);
bool ble_worker_event_handler_read_request_event(size_t data_size, void* data, void* context);
