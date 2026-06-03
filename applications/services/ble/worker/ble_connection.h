#pragma once

#include <furi.h>

typedef struct BleConnectionContext BleConnectionContext;

BleConnectionContext* ble_connection_alloc(const uint8_t* const peer_address);
void ble_connection_free(BleConnectionContext* instance);
