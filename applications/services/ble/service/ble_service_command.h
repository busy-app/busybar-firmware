#pragma once

typedef enum {
    BleServiceCommandUnknown,
    BleServiceCommandInit,
    BleServiceCommandDeinit,
    BleServiceCommandRun,
    BleServiceCommandUpdate,

    BleServiceCommandCount,
} BleServiceCommandEnum;
