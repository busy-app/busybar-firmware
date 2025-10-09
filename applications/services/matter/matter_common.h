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
