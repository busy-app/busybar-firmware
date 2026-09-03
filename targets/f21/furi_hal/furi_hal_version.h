/**
 * @file furi_hal_version.h
 * Version HAL API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <version/version.h>
#include <furi.h>
#include <furi_hal_flash_otp.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FURI_HAL_VERSION_NAME_LENGTH       (8)
#define FURI_HAL_VERSION_ARRAY_NAME_LENGTH (FURI_HAL_VERSION_NAME_LENGTH + 1)
#define FURI_HAL_VERSION_MAC_LENGTH        (6)

/** Device Colors (from OTP2 hw_color, matches HWColor in bsbotp.py) */
typedef enum {
    FuriHalVersionColorUnknown = 0x00,
    FuriHalVersionColorWhite = 0x01,
} FuriHalVersionColor;

/** Init flipper version
 */
void furi_hal_version_init(void);

/** Check target firmware version
 *
 * @return     true if target and real matches
 */
bool furi_hal_version_check_target_match(void);

/** Get model name
 *
 * @return     model name C-string
 */
const char* furi_hal_version_get_model_name(void);

/** Get model name
 *
 * @return     model code C-string
 */
const char* furi_hal_version_get_model_code(void);

/** Get FCC ID
 *
 * @return     FCC id as C-string
 */
const char* furi_hal_version_get_fcc_id(void);

/** Get IC id
 *
 * @return     IC id as C-string
 */
const char* furi_hal_version_get_ic_id(void);

/** Get MIC id
 *
 * @return     MIC id as C-string
 */
const char* furi_hal_version_get_mic_id(void);

/** Get SRRC id
 *
 * @return     SRRC id as C-string
 */
const char* furi_hal_version_get_srrc_id(void);

/** Get NCC id
 *
 * @return     NCC id as C-string
 */
const char* furi_hal_version_get_ncc_id(void);

/** Get hardware version
 *
 * @return     Hardware Version
 */
uint8_t furi_hal_version_get_hw_version(void);

/** Get hardware target (with fallback to firmware version if OTP not provisioned)
 *
 * @return     Hardware Target
 */
uint8_t furi_hal_version_get_hw_target(void);

/** Get hardware target from OTP (raw value, 0 if not provisioned)
 *
 * @return     Hardware Target from OTP, or 0 if OTP1 not provisioned
 */
uint8_t furi_hal_version_get_hw_target_otp(void);

/** Get hardware body
 *
 * @return     Hardware Body
 */
uint8_t furi_hal_version_get_hw_body(void);

/** Get hardware body color
 *
 * @return     Hardware Color
 */
FuriHalVersionColor furi_hal_version_get_hw_color(void);

/** Get hardware connect
 *
 * @return     Hardware Interconnect
 */
uint8_t furi_hal_version_get_hw_connect(void);

/** Get full hardware version code
 * 
 * @return     Version code (like `"4.F22.B7.C2"`).
 *             Value is `"Unknown"` if OTP1 not provisioned.
 */
const char* furi_hal_version_get_hw_version_code(void);

/** Get hardware region (raw value from OTP2)
 *
 * @return     Hardware Region value
 */
uint8_t furi_hal_version_get_hw_region(void);

/** Get hardware timestamp
 *
 * @return     Hardware Manufacture timestamp
 */
uint32_t furi_hal_version_get_hw_timestamp(void);

/** Get QC timestamp from OTP2
 *
 * @return     QC timestamp, or 0 if not provisioned
 */
uint32_t furi_hal_version_get_hw_timestamp_qc(void);

/** Get pointer to target name
 *
 * @return     Hardware Name C-string
 */
const char* furi_hal_version_get_name_ptr(void);

/** Get USB MAC address
 *
 * @return     pointer to USB MAC address array
 *             (length of `FURI_HAL_VERSION_MAC_LENGTH`)
 */
const uint8_t* furi_hal_version_get_usb_mac(void);

/** Get platform UID size in bytes
 *
 * @return     UID size in bytes
 */
size_t furi_hal_version_uid_size(void);

/** Get const pointer to UID
 *
 * @return     pointer to UID
 */
const uint8_t* furi_hal_version_uid(void);

// ============================================================================
// OTP Validity Status
// ============================================================================

/** Check if an OTP block is valid (has magic and is programmed)
 *
 * @param      block  OTP block identifier
 * @return     true if the specified OTP block is valid
 */
bool furi_hal_version_is_otp_valid(FuriHalFlashOtpBlock block);

// ============================================================================
// OTP3 - Public Key Data
// ============================================================================

typedef enum {
    FuriHalVersionKeyCurveNone = 0x00,
    FuriHalVersionKeyCurveSecp224r1 = 0x01,
} FuriHalVersionKeyCurve;

/** Get EC curve identifier from OTP3
 *
 * @return     Curve ID
 */
FuriHalVersionKeyCurve furi_hal_version_get_sign_curve(void);

/** Get public key from OTP3
 *
 * @return     Pointer to 56-byte public key (X || Y), or NULL if not provisioned
 */
const uint8_t* furi_hal_version_get_sign_pubkey(void);

/** Get public key size
 *
 * @return     Public key size in bytes (56)
 */
size_t furi_hal_version_get_sign_pubkey_size(void);

// ============================================================================
// OTP4 - Signature Data
// ============================================================================

/** Get stored MCU UID from OTP4 (for binding verification)
 *
 * @return     Pointer to 12-byte MCU UID, or NULL if not provisioned
 */
const uint8_t* furi_hal_version_get_otp_mcu_uid(void);

/** Get OTP1 signature from OTP4
 *
 * @return     Pointer to 56-byte signature, or NULL if not provisioned
 */
const uint8_t* furi_hal_version_get_otp1_signature(void);

/** Get OTP2 signature from OTP4
 *
 * @return     Pointer to 56-byte signature, or NULL if not provisioned
 */
const uint8_t* furi_hal_version_get_otp2_signature(void);

/** Get signature size
 *
 * @return     Signature size in bytes (56)
 */
size_t furi_hal_version_get_signature_size(void);

#ifdef __cplusplus
}
#endif
