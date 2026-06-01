#pragma once

typedef enum {
    BleIncomingNwpEventTypeUnknown,
    BleIncomingNwpEventTypeExit,
    BleIncomingNwpEventTypeAdvReport,
    BleIncomingNwpEventTypeConnected,
    BleIncomingNwpEventTypeDisconnected,
    BleIncomingNwpEventTypePhyUpdateComplete,
    BleIncomingNwpEventTypeConnUpdate,
    BleIncomingNwpEventTypeDataLengthChange,

    BleIncomingNwpEventTypeReceiveRemoteFeatures,
    BleIncomingNwpEventTypeReadRequest,
    BleIncomingNwpEventTypeMoreDataRequest,

    BleIncomingNwpEventTypeWrite,
    BleIncomingNwpEventTypeDataTransmit,
    BleIncomingNwpEventTypeMtu,

    BleIncomingNwpEventTypeSmpResponse,
    BleIncomingNwpEventTypeSmpEncryptStarted,
    BleIncomingNwpEventTypeSmpLtkRequest,
    BleIncomingNwpEventTypeSmpSecurityKeys,
    BleIncomingNwpEventTypeSmpPairingFailed,
    BleIncomingNwpEventTypeAdjustConnectionRequest,

    BleIncomingNwpEventTypeCount,
} BleIncomingNwpEventType;
