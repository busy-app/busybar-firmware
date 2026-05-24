#pragma once

#include "ble_incoming_nwp_event_type_enum.h"
#include <furi.h>

typedef struct {
    BleIncomingNwpEventType type;
    size_t data_size;
    void* data;
} BleWorkerEvent;

typedef FuriMessageQueue* BleEventQueuePtr;

typedef bool (*BleWorkerEventHandler)(size_t data_size, void* data, void* context);

void ble_worker_spawn_event(
    BleEventQueuePtr queue,
    BleIncomingNwpEventType type,
    size_t data_size,
    void* data);

void ble_worker_process_event(
    BleEventQueuePtr queue,
    const BleWorkerEventHandler* const event_handlers,
    void* context);

///TODO: need to find more elegant solution. Maybe skip all events, if we are exiting;
void ble_worker_flush_events(BleEventQueuePtr queue);
