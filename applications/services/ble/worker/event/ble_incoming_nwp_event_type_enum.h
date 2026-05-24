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
    BleIncomingNwpEventTypeMoreDataRequest,

    BleIncomingNwpEventTypeWrite,
    BleIncomingNwpEventTypeDataTransmit,
    BleIncomingNwpEventTypeMtu,
    BleIncomingNwpEventTypeIndicateConfirm,

    BleIncomingNwpEventTypeSmpResponse,
    BleIncomingNwpEventTypeSmpEncryptStarted,
    BleIncomingNwpEventTypeSmpLtkRequest,
    BleIncomingNwpEventTypeSmpSecurityKeys,
    BleIncomingNwpEventTypeSmpPairingFailed,
    BleIncomingNwpEventTypeAdjustConnectionRequest,

    BleIncomingNwpEventTypeCount,
} BleIncomingNwpEventType;
