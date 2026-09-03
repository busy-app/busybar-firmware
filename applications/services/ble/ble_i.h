#pragma once

#include "ble.h"
#include "ble_log.h"
#include "ble_command_engine.h"
#include "service/ble_service.h"
#include "service/ble_service_config.h"

#include <intercom/intercom.h>
#include <furi.h>

#if !defined(BSB_MCU_SI917)
#include "streaming/ble_streaming.h"
#else
#include "worker/ble_worker.h"
#endif

typedef enum {
    BleEventTypeFrameReceived = (1 << 0),
    BleEventTypeFrameLost = (1 << 1),
    BleEventTypeIntercomInit = (1 << 2),
    BleEventTypeIntercomDeinit = (1 << 3),
} BleEventType;

typedef void (
    *BleServicePostProcessCallback)(BleServiceObject* service, bool result, void* extra_context);

struct Ble {
    BleServiceStatus status;
    FuriMutex* ble_lock;
    FuriSemaphore* mailbox_lock;
    BleIntercomFrameGeneric mailbox;
    BleCommandEngine* engine;

    FuriMessageQueue* service_queue;
    FuriEventLoop* event_loop;
    Intercom* intercom;
    IntercomChannel* intercom_ch;
    FuriString* error;

    BleServiceObject* services[BleServiceIndexCount];
    uint8_t remote_device_address[BLE_REMOTE_DEVICE_ADDRESS_STRING_SIZE];
#if !defined(BSB_MCU_SI917)
    BleStreaming* streaming;
    FuriPubSub* on_status_change;
#else
    BleWorker* worker;
#endif
    BleServicePostProcessCallback service_post_process_callback;
};

void ble_set_service_post_process_callback(Ble* ble, BleServicePostProcessCallback callback);
