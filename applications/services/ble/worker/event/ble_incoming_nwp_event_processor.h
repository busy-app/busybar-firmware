#pragma once

#include "ble_incoming_nwp_event_type_enum.h"
#include <furi.h>

typedef struct BleIncomingNwpEventProcessor BleIncomingNwpEventProcessor;

///TODO: move to .C file
typedef bool (*BleWorkerEventHandler)(size_t data_size, void* data, void* context);

BleIncomingNwpEventProcessor* ble_incoming_nwp_event_processor_alloc();

void ble_incoming_nwp_event_processor_run(
    BleIncomingNwpEventProcessor* instance,
    FuriEventLoop* event_loop);

void ble_incoming_nwp_event_processor_spawn_event(
    BleIncomingNwpEventProcessor* instance,
    BleIncomingNwpEventType type,
    size_t data_size,
    void* data);

// void ble_incoming_nwp_event_processor_process_event(
//     BleEventQueuePtr queue,
//     const BleWorkerEventHandler* const event_handlers,
//     void* context);

// ///TODO: need to find more elegant solution. Maybe skip all events, if we are exiting;
// void ble_incoming_nwp_event_processor_flush_events(BleEventQueuePtr queue);
