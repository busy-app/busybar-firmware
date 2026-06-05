#pragma once

#include <furi.h>

bool ble_worker_event_handler_advertise_report(size_t data_size, void* data, void* context);
bool ble_worker_event_handler_connected(size_t data_size, void* data, void* context);
bool ble_worker_event_handler_disconnected(size_t data_size, void* data, void* context);
bool ble_worker_event_handler_phy_update_complete(size_t data_size, void* data, void* context);
bool ble_worker_event_handler_connection_update(size_t data_size, void* data, void* context);
bool ble_worker_event_handler_length_change(size_t data_size, void* data, void* context);
bool ble_worker_event_handler_receive_remote_features(size_t data_size, void* data, void* context);
bool ble_worker_event_handler_more_data_request(size_t data_size, void* data, void* context);

bool ble_worker_event_handler_exit(size_t data_size, void* data, void* context);
bool ble_worker_event_handler_adjust_connection_request(
    size_t data_size,
    void* data,
    void* context);
