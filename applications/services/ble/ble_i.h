#pragma once

#include "ble.h"
#include "ble_common.h"
#include "ble_command_engine.h"
#include "service/ble_service.h"
#include "service/ble_service_config.h"

#include <intercom/intercom.h>
#include <furi.h>

#if !defined(BSB_MCU_SI917)

#include <api_lock.h>

typedef struct {
    BleIntercomFrameHeader header;
    uint8_t data[];
} BleCommand;

#endif

typedef enum {
    BleEventTypeInitOnStart = (1 << 0),
    BleEventTypeEnableOnStart = (1 << 1),
    BleEventTypeIncomingMessage = (1 << 2),
    BleEventTypeFrameReceived = (1 << 3),
    BleEventTypeDeviceNameChanged = (1 << 4),
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
    IntercomChannel* intercom_ch;
    //--------------------------
    FuriString* error;

    BleServiceObject* services[BLE_SERVICES_COUNT];
    uint8_t remote_device_address[BLE_REMOTE_DEVICE_ADDRESS_STRING_SIZE];
#if !defined(BSB_MCU_SI917)
    FuriPubSub* on_status_change;
    FuriApiLock current_command_api_lock;
    FuriMutex* current_command_lock;
    BleCommand* current_command;
    size_t current_command_size;

    BleServicePostProcessCallback service_post_process_callback;
#endif
};

#if !defined(BSB_MCU_SI917)
void ble_set_service_post_process_callback(Ble* ble, BleServicePostProcessCallback callback);
#endif
