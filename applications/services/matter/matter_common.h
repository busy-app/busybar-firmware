/**
 * @file matter_common.h
 * Abstract definitions.
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MATTER_COMMISSION_TIME_SECONDS (60 * 15)

/** Spec paragraph 5.1.3 */
#define MATTER_QR_CODE_LEN_MAX (255)

/** Spec paragraph 5.1.4 says 21. We also add a maximum of 5 hyphens. */
#define MATTER_MAN_CODE_LEN_MAX (26)

typedef enum {
    MatterStatusOk,
    MatterStatusTimeout,
    MatterStatusFsError,
    MatterStatusBadConfig,
    MatterStatusUnprovisioned,
    MatterStatusMax,
} MatterStatus;

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

typedef struct {
    char qr_code[MATTER_QR_CODE_LEN_MAX + 1];
    char manual_code[MATTER_MAN_CODE_LEN_MAX + 1];
    uint32_t window_duration_s;
} MatterCommissioningInfo;

#ifdef __cplusplus
}
#endif
