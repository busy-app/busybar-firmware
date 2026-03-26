/**
 * @file matter_common.h
 * Abstract definitions.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Node commissioning (onboarding) status
 */
typedef enum {
    MatterCommissioningStatusNeverStarted,
    MatterCommissioningStatusStarted,
    MatterCommissioningStatusComplete,
    MatterCommissioningStatusFailed,
    MatterCommissioningStatusMAX,
} MatterCommissioningStatus;

/**
 * @brief Enumeration of possible startup modes for the Matter switch
 */
typedef enum {
    MatterSwitchStartupModeOff, /**< Set switch to OFF at startup */
    MatterSwitchStartupModeOn, /**< Set switch to ON at startup */
    MatterSwitchStartupModeToggle, /**< Toggle switch at startup */
    MatterSwitchStartupModeLast, /**< Set switch to last state at startup */
    MatterSwitchStartupModeMAX, /**< Special value, internal use */
} MatterSwitchStartupMode;

typedef enum {
    MatterSwitchStateUnknown,
    MatterSwitchStateOff,
    MatterSwitchStateOn,
    MatterSwitchStateMax,
} MatterSwitchState;

#ifdef __cplusplus
}
#endif
