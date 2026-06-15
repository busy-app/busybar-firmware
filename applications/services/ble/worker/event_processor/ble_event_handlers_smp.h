#pragma once

#include <furi.h>

bool ble_event_handler_smp_response(size_t data_size, void* data, void* context);
bool ble_event_handler_smp_encrypt_started(size_t data_size, void* data, void* context);
bool ble_event_handler_smp_ltk_request(size_t data_size, void* data, void* context);
bool ble_event_handler_smp_security_keys(size_t data_size, void* data, void* context);
bool ble_event_handler_smp_pairing_failed(size_t data_size, void* data, void* context);
