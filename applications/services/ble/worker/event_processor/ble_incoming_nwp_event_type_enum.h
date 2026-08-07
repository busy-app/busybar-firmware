/**
 * @file ble_incoming_nwp_event_type_enum.h
 * @brief Events spawned when any nwp callback is triggered
 */
#pragma once

/**
 * @brief Enumeration of all events which can be spawned in @ref "ble_nwp_core_callbacks.h" module
 * by @ref "ble_incoming_nwp_event_processor.h"  also some events can be spawned internally 
 * in order to perform some actions
 */
typedef enum {
    BleIncomingNwpEventTypeUnknown,
    BleIncomingNwpEventTypeExit,
    BleIncomingNwpEventTypeForgetPaired,

    BleIncomingNwpEventTypeConnected,
    BleIncomingNwpEventTypeDisconnected,
    BleIncomingNwpEventTypePhyUpdateComplete,
    BleIncomingNwpEventTypeConnUpdate,
    BleIncomingNwpEventTypeDataLengthChange,

    BleIncomingNwpEventTypeReceiveRemoteFeatures,
    BleIncomingNwpEventTypeReadRequest,

    BleIncomingNwpEventTypeWrite,
    BleIncomingNwpEventTypeDataTransmit,
    BleIncomingNwpEventTypeMtu,

    BleIncomingNwpEventTypeSmpResponse,
    BleIncomingNwpEventTypeSmpEncryptStarted,
    BleIncomingNwpEventTypeSmpLtkRequest,
    BleIncomingNwpEventTypeSmpSecurityKeys,
    BleIncomingNwpEventTypeSmpPairingFailed,

    BleIncomingNwpEventTypeCount,
} BleIncomingNwpEventType;
