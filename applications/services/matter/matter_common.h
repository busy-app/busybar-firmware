/**
 * @file matter_common.h
 * Abstract definitions.
 */

#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Identifier for a virtual device
 */
typedef enum {
    MatterVirtualDeviceSwitch1, //<! Switch 1. State type: bool
    MatterVirtualDeviceSwitch2, //<! Switch 2. State type: bool
    // TODO: Switch2
    MatterVirtualDeviceMAX, //<! Do not use
} MatterVirtualDevice;

/**
 * @brief State of a virtual device along with its identifier
 * 
 * @note This type includes the device ID to make it very obvious (at least when
 * constructing the value) what union field one should use.
 */
typedef struct {
    MatterVirtualDevice device;
    union {
        bool bool_val;
    };
} MatterVirtualDeviceState;

/**
 * @brief Node commissioning (onboarding) status
 */
typedef enum {
    MatterCommissioningStatusStarted,
    MatterCommissioningStatusComplete,
    MatterCommissioningStatusFailed,
    MatterCommissioningStatusMAX,
} MatterCommissioningStatus;

#ifdef __cplusplus
}
#endif
