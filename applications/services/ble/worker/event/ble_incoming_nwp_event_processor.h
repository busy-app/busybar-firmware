#pragma once

#include "ble_incoming_nwp_event_type_enum.h"
#include <furi.h>

typedef struct BleIncomingNwpEventProcessor BleIncomingNwpEventProcessor;

///TODO: Cleanup this, remove semaphores
BleIncomingNwpEventProcessor* ble_incoming_nwp_event_processor_alloc(
    void* context,
    FuriSemaphore* transmit_sem,
    FuriSemaphore* indicate_sem);

///TODO: event_loop should be from outside, or this should be replaced to
///subscribe method
void ble_incoming_nwp_event_processor_run(
    BleIncomingNwpEventProcessor* instance,
    FuriEventLoop* event_loop);

void ble_incoming_nwp_event_processor_spawn_event(
    BleIncomingNwpEventProcessor* instance,
    BleIncomingNwpEventType type,
    size_t data_size,
    void* data);
