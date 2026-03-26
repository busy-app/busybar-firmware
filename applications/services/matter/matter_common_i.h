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

#define MATTER_INTERCOM_PROTOCOL_VERSION (1)

#define MATTER_COMMISSION_TIME_SECONDS (60 * 15)
#define MATTER_MAX_QR_CODE_LEN         (255) // Spec paragraph 5.1.3
#define MATTER_MAX_MAN_CODE_LEN \
    (26) // Spec paragraph 5.1.4 says 21. We also add a maximum of 5 hyphens.

// ===============
// Specific frames
// ===============

/**
 * @brief Initialization parameters
 */
typedef struct {
    uint8_t hardware_version_num;
    char hardware_version_str[20];
    uint16_t cd_certificate_length;
    uint8_t cd_certificate[512];
} MatterIntercomInitializationFrame;

/**
 * @brief Request to change state
 */
typedef struct {
    MatterSwitchState state;
} MatterIntercomSwitchStateFrame;

/**
 * @brief Request to change startup mode
 */
typedef struct {
    MatterSwitchStartupMode mode;
} MatterIntercomStartupModeFrame;

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
    MatterIntercomFrameTypeInitialization, //<! Initialization parameters. Direction: u5->917
    MatterIntercomFrameTypeBackendReady, //<! Backend has fully initialized. Direction: 917->u5
    MatterIntercomFrameTypeSwitchState, //<! Request to change state. Direction: u5<->917
    MatterIntercomFrameTypeSwitchStartupMode, //<! Request to change startup mode. Direction: u5->917
    MatterIntercomFrameTypeReset, //<! Factory reset. Direction: u5->917
    MatterIntercomFrameTypeCommission, //<! Enter commissioning mode. Direction: u5->917
    MatterIntercomFrameTypePairingCodes, //<! Pairing codes. Direction: 917->u5
    MatterIntercomFrameTypeCommissionStatus, //<! Commissioning status update. Direction: 917->u5
    MatterIntercomFrameTypeFabricCountUpdate, // <! Commissioned fabric count. Direction: 917->u5
    MatterIntercomFrameTypeMax,
} MatterIntercomFrameType;

typedef struct {
    MatterIntercomFrameType type;
    union {
        uint8_t frame_of_any_type;
        MatterIntercomInitializationFrame initialization;
        MatterIntercomSwitchStateFrame switch_state;
        MatterIntercomStartupModeFrame startup;
        MatterIntercomPairingCodesFrame codes;
        MatterIntercomCommissionStatusFrame commission_status;
        MatterIntercomFabricCountUpdateFrame fabric_count;
    };
} MatterIntercomFrame;

#ifdef __cplusplus
}
#endif
