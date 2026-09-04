/**
 * @file ble_service_command.h
 * @brief Enumeration of services commands
 */
#pragma once

typedef enum {
    BleServiceCommandUnknown,
    BleServiceCommandInit, /**< Init service */
    BleServiceCommandDeinit, /**< Deinit service, called internally on failure only */
    BleServiceCommandRun, /**< Run command, called when service needs to be processed */
    BleServiceCommandUpdate, /**< Data update from another side */

    BleServiceCommandCount,
} BleServiceCommandEnum;
