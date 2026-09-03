#pragma once

#include "ble_transmitter.h"

#include "../_nwp_callbacks/ble_nwp_headers.h"
#include "../../ble_log.h"

typedef enum {
    BleTransmitterTypeIndication,
    BleTransmitterTypeSet,

    BleTransmitterTypeCount,
} BleTransmitterType;

typedef void BleTransmitterGeneric;

struct BleTransmitter {
    BleTransmitterGeneric* context[BleTransmitterTypeCount];
};
