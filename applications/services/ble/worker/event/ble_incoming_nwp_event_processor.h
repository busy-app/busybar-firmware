#pragma once

#include "ble_incoming_nwp_event_type_enum.h"
#include <furi.h>

typedef struct BleIncomingNwpEventProcessor BleIncomingNwpEventProcessor;

BleIncomingNwpEventProcessor* ble_incoming_nwp_event_processor_alloc(void* context);

void ble_incoming_nwp_event_processor_subscribe(
    BleIncomingNwpEventProcessor* instance,
    FuriEventLoop* event_loop);

void ble_incoming_nwp_event_processor_unsubscribe(
    BleIncomingNwpEventProcessor* instance,
    FuriEventLoop* event_loop);

void ble_incoming_nwp_event_processor_spawn_event(
    BleIncomingNwpEventProcessor* instance,
    BleIncomingNwpEventType type,
    size_t data_size,
    void* data);
