#pragma once

#include <furi.h>

typedef enum {
    BleWorkerEventTypeUnknown,
    BleWorkerEventTypeExit,
    BleWorkerEventTypeAdvReport,
    BleWorkerEventTypeConnected,
    BleWorkerEventTypeDisconnected,
    BleWorkerEventTypePhyUpdateComplete,
    BleWorkerEventTypeConnUpdate,
    BleWorkerEventTypeDataLengthChange,

    BleWorkerEventTypeReceiveRemoteFeatures,
    BleWorkerEventTypeMoreDataRequest,

    BleWorkerEventTypeWrite,
    BleWorkerEventTypeDataTransmit,
    BleWorkerEventTypeMtu,
    BleWorkerEventTypeIndicateConfirm,

    BleWorkerEventTypeSmpResponse,
    BleWorkerEventTypeSmpEncryptStarted,
    BleWorkerEventTypeSmpLtkRequest,
    BleWorkerEventTypeSmpSecurityKeys,
    BleWorkerEventTypeSmpPairingFailed,
    BleWorkerEventTypeAdjustConnectionRequest,

    BleWorkerEventTypeCount,
} BleWorkerEventType;

typedef struct {
    BleWorkerEventType type;
    size_t data_size;
    void* data;
} BleWorkerEvent;

typedef FuriMessageQueue* BleEventQueuePtr;

typedef bool (*BleWorkerEventHandler)(size_t data_size, void* data, void* context);

void ble_worker_spawn_event(
    BleEventQueuePtr queue,
    BleWorkerEventType type,
    size_t data_size,
    void* data);

void ble_worker_process_event(
    BleEventQueuePtr queue,
    const BleWorkerEventHandler* const event_handlers,
    void* context);

///TODO: need to find more elegant solution. Maybe skip all events, if we are exiting;
void ble_worker_flush_events(BleEventQueuePtr queue);
