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
    BleIntercomFrameHeader header;
    uint8_t data[];
} BleMessage;

#endif

typedef enum {
    BleEventTypeIncomingMessage = (1 << 0),
    BleEventTypeFrameReceived = (1 << 1),
    BleEventTypeDeviceNameChanged = (1 << 3),
} BleEventType;

typedef void (
    *BleServicePostProcessCallback)(BleServiceObject* service, bool result, void* extra_context);

struct Ble {
    BleServiceState state;
    FuriMutex* ble_lock;
    FuriSemaphore* mailbox_lock;
    BleIntercomFrameGeneric mailbox;
    BleCommandEngine* engine;

    FuriMessageQueue* message_queue;
    FuriEventLoop* event_loop;
    Intercom* intercom;
    //--------------------------
    FuriString* error;

    BleServiceObject* services[BLE_SERVICES_COUNT];
#if !defined(SI917)
    FuriApiLock current_message_api_lock;
    FuriMutex* current_message_lock;
    BleMessage* current_message;
    size_t current_message_size;

    BleServicePostProcessCallback service_post_process_callback;
#endif
};

bool ble_init(Ble* ble);

#if !defined(SI917)
void ble_set_service_post_process_callback(Ble* ble, BleServicePostProcessCallback callback);
#endif
