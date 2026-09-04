/**
 * @file ble_service_index.h
 * @brief Enumeration of services available on the device 
 */
#pragma once

#include <furi.h>

/**
 * @brief Enumeration of services available on the device
 * used in various methods for accessing particular service
 */
typedef enum {
    BleServiceIndexGenericAccess,
    BleServiceIndexGenericAttribute,
    BleServiceIndexDeviceInfo,
    BleServiceIndexBattery,
    BleServiceIndexNordicUart,
    BleServiceIndexHm10Uart,

    BleServiceIndexCount
} BleServiceIndex;
