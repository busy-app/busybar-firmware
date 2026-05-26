#pragma once

#include <furi.h>

typedef enum {
    BleServiceIndexGenericAccess,
    BleServiceIndexGenericAttribute,
    BleServiceIndexDeviceInfo,
    BleServiceIndexBattery,
    BleServiceIndexNordicUart,
    BleServiceIndexHm10Uart,

    BleServiceIndexCount
} BleServiceIndex;
