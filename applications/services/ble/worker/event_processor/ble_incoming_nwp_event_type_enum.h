#pragma once

typedef enum {
    BleIncomingNwpEventTypeUnknown,
    BleIncomingNwpEventTypeExit,

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
