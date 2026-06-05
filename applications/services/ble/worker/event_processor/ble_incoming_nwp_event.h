#pragma once

#include "ble_incoming_nwp_event_type_enum.h"

#include <furi.h>

typedef struct {
    BleIncomingNwpEventType type;
    size_t data_size;
    void* data;
} BleIncomingNwpEvent;

BleIncomingNwpEvent*
    ble_incoming_nwp_event_alloc(BleIncomingNwpEventType type, size_t data_size, void* data);

void ble_incoming_nwp_event_free(BleIncomingNwpEvent* instance);
