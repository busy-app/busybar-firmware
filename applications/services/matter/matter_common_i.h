/**
 * @file matter_common_i.h
 * Intercom protocol definitions.
 */

#pragma once

#include <furi.h>
#include "matter_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// ===============
// Specific frames
// ===============

/**
 * @brief Notification about updated state
 */
typedef struct {
    MatterVirtualDeviceState new_state;
} MatterIntercomUpdateFrame;

/**
 * @brief Request to change state
 */
typedef struct {
    MatterVirtualDeviceState req_state;
} MatterIntercomRequestFrame;

/**
 * @brief Request to wipe all Matter-related data (factory reset)
 */
typedef struct {
} MatterIntercomResetFrame;

// =============
// Generic frame
// =============

typedef enum {
    MatterIntercomFrameTypeUpdate, //<! Notification about updated state. Direction: 917->u5
    MatterIntercomFrameTypeRequest, //<! Request to change state. Direction: u5->917
    MatterIntercomFrameTypeReset, //<! Factory reset. Direction: u5->917
} MatterIntercomFrameType;

typedef struct {
    MatterIntercomFrameType type;
    union {
        MatterIntercomUpdateFrame update;
        MatterIntercomRequestFrame request;
        MatterIntercomResetFrame reset;
    };
} MatterIntercomFrame;

#ifdef __cplusplus
}
#endif
