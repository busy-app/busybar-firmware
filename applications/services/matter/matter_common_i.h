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

#define MATTER_COMMISSION_TIME_SECONDS (60 * 15)
#define MATTER_MAX_QR_CODE_LEN         (255) // Spec paragraph 5.1.3
#define MATTER_MAX_MAN_CODE_LEN \
    (26) // Spec paragraph 5.1.4 says 21. We also add a maximum of 5 hyphens.

// ===============
// Specific frames
// ===============

/**
 * @brief Notification about updated state
 */
typedef struct {
    bool state;
} MatterIntercomUpdateFrame;

/**
 * @brief Request to change state
 */
typedef struct {
    bool switch_state;
} MatterIntercomRequestFrame;

/**
 * @brief Request to wipe all Matter-related data (factory reset)
 */
typedef struct {
} MatterIntercomResetFrame;

/**
 * @brief Request to open basic commissioning window
 */
typedef struct {
} MatterIntercomCommissionFrame;

/**
 * @brief Number of commissioned fabrics
 */
typedef struct {
    uint8_t fabric_count;
} MatterIntercomFabricCountUpdateFrame;

/**
 * @brief Pairing codes
 */
typedef struct {
    char qr_code[MATTER_MAX_QR_CODE_LEN + 1];
    char manual_code[MATTER_MAX_MAN_CODE_LEN + 1];
} MatterIntercomPairingCodesFrame;

typedef struct {
    MatterCommissioningStatus status;
} MatterIntercomCommissionStatusFrame;

// =============
// Generic frame
// =============

typedef enum {
    MatterIntercomFrameTypeUpdate, //<! Notification about updated state. Direction: 917->u5
    MatterIntercomFrameTypeRequest, //<! Request to change state. Direction: u5->917

    MatterIntercomFrameTypeReset, //<! Factory reset. Direction: u5->917

    MatterIntercomFrameTypeCommission, //<! Enter commissioning mode. Direction: u5->917
    MatterIntercomFrameTypePairingCodes, //<! Pairing codes. Direction: 917->u5
    MatterIntercomFrameTypeCommissionStatus, //<! Commissioning status update. Direction: 917->u5

    MatterIntercomFrameTypeFabricCountUpdate, // <! Commissioned fabric count. Direction: 917->u5
} MatterIntercomFrameType;

typedef struct {
    MatterIntercomFrameType type;
    union {
        uint8_t frame_of_any_type;
        MatterIntercomUpdateFrame update;
        MatterIntercomRequestFrame request;
        MatterIntercomResetFrame reset;
        MatterIntercomCommissionFrame commission;
        MatterIntercomPairingCodesFrame codes;
        MatterIntercomCommissionStatusFrame commission_status;
        MatterIntercomFabricCountUpdateFrame fabric_count;
    };
} MatterIntercomFrame;

#ifdef __cplusplus
}
#endif
