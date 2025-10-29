#pragma once

#include "ble.h"
#include "ble_common.h"
#include "ble_command_engine.h"
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
    BleEventTypeIncomingMessage = (1 << 0),
    BleEventTypeFrameReceived = (1 << 1),
    BleEventTypeServiceStateChanged = (1 << 2),
} BleEventType;

struct Ble {
    BleServiceState state;
    FuriMutex* ble_lock;
    FuriSemaphore* mailbox_lock;
    BleIntercomFrameGeneric mailbox;
    BleCommandEngine* engine;

    FuriMessageQueue* message_queue;
    FuriEventLoop* event_loop;
    IntercomChannel* intercom_ch;
    //--------------------------

    BleServiceObject* services[BLE_SERVICES_COUNT];
#if !defined(SI917)
    FuriTimer* init_timer;
    BleMessage* current_message;
#endif
};

bool ble_init(Ble* ble);
