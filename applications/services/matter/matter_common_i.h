/**
 * @file matter_common_i.h
 * Intercom protocol definitions.
 */
#pragma once

#include "matter_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t data[512];
    uint16_t length;
} MatterCertificateDeclaration;

// ===============
// Specific frames
// ===============

/**
 * @brief Initialization parameters
 */
typedef struct {
    uint8_t hardware_version_num;
    char hardware_version_str[20];
    MatterCertificateDeclaration cd;
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
    char qr_code[MATTER_QR_CODE_LEN_MAX + 1];
    char manual_code[MATTER_MAN_CODE_LEN_MAX + 1];
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
