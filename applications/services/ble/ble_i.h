#pragma once

#include "ble.h"
#include "ble_common.h"
#include "service/ble_service.h"
#include "service/ble_service_config.h"

#include <intercom/intercom.h>
#include <furi.h>

#if !defined(SI917)

#include <api_lock.h>

typedef struct {
    bool result; ///TODO: replace with some more extended status
    FuriApiLock lock;
    BleIntercomFrameHeader header;
    uint8_t data[];
} BleMessage;

#endif

typedef enum {
    BleEventTypeIncomingMessage,
    BleEventTypeFrameReceived,
} BleEventType;

struct Ble {
    BleServiceState state;

    FuriMessageQueue* message_queue;

    FuriSemaphore* mailbox_lock;
    BleIntercomFrameGeneric mailbox;

    FuriEventLoopTimer* init_timer;
    // FuriEventLoopTimer* test_timer;
    FuriMutex* ble_lock;

    FuriEventLoop* event_loop;
    Intercom* intercom;
    //--------------------------

    FuriSemaphore* access_semaphore;

    BleServiceObject* services[BLE_SERVICES_COUNT];
#if !defined(SI917)
    BleMessage* current_message;
#endif
};
