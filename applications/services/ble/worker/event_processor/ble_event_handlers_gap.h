#pragma once

#include <furi.h>

bool ble_event_handler_gap_advertise_report(size_t data_size, void* data, void* context);
bool ble_event_handler_gap_connected(size_t data_size, void* data, void* context);
bool ble_event_handler_gap_disconnected(size_t data_size, void* data, void* context);
bool ble_event_handler_gap_phy_update_complete(size_t data_size, void* data, void* context);
bool ble_event_handler_gap_connection_update(size_t data_size, void* data, void* context);
bool ble_event_handler_gap_length_change(size_t data_size, void* data, void* context);
bool ble_event_handler_gap_receive_remote_features(size_t data_size, void* data, void* context);

bool ble_event_handler_gap_exit(size_t data_size, void* data, void* context);
