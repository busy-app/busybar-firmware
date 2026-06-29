#pragma once

#include "ble_worker.h"

#include "device/ble_device.h"
#include "event_processor/ble_incoming_nwp_event_processor.h"
#include <api_lock.h>

struct BleWorker {
    FuriThread* thread;
    FuriEventLoop* event_loop;
    FuriApiLock api_lock;
    BleIncomingNwpEventProcessor* event_proc;
    BleTransmitter* transport;
    BleDevice* device;

    BleConnectionStateChanged on_connection_changed_cb;
    void* on_connection_changed_ctx;
};

void ble_worker_invoke_connect_callback(BleWorker* instance);

void ble_worker_invoke_disconnect_callback(BleWorker* instance);
