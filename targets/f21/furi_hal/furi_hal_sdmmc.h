#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <furi/core/common_defines.h>

typedef enum {
    FuriHalSdVersion1,
    FuriHalSdVersion2,
} FuriHalSdVersion;

typedef enum {
    FuriHalSdTypeSC, /*!< SD Standard Capacity <2G */
    FuriHalSdTypeHCXC, /*!< SD High Capacity <32G, SD Extended Capacity <2T */
    FuriHalSdTypeMMCLowCapacity, /*!< MMC Low Capacity */
    FuriHalSdTypeMMCHighCapacity, /*!< MMC High Capacity */
} FuriHalSdType;

typedef enum {
    FuriHalSdSpeedNormal, /*!< Normal Speed Card <12.5Mb/s, Spec Version 1.01 */
    FuriHalSdSpeedHigh, /*!< High Speed Card <25Mb/s , Spec version 2.00 */
    FuriHalSdSpeedUltraHigh, /*!< UHS-I SD Card <50Mb/s for SDR50, DDR5 Cards and <104Mb/s for SDR104, Spec version 3.01 */
} FuriHalSdSpeed;

typedef struct {
    uint32_t logical_block_count; /*!< logical capacity in blocks */
    uint32_t logical_block_size; /*!< logical block size in bytes */

    FuriHalSdVersion version; /*!< SD version */
    FuriHalSdType type; /*!< SD type */
    FuriHalSdSpeed speed; /*!< SD speed */

    uint8_t manufacturer_id; /*!< manufacturer ID */
    char oem_id[3]; /*!< OEM ID, 2 characters + null terminator */
    char product_name[6]; /*!< product name, 5 characters + null terminator */
    uint8_t product_revision_major; /*!< product revision major */
    uint8_t product_revision_minor; /*!< product revision minor */
    uint32_t product_serial_number; /*!< product serial number */
    uint8_t manufacturing_month; /*!< manufacturing month */
    uint16_t manufacturing_year; /*!< manufacturing year */
} FuriHalSdInfo;

typedef enum _FuriHalSdError {
    _FuriHalSdErrorNone = 0,
    _FuriHalSdErrorDataCrcFail,
    _FuriHalSdErrorDataTimeout,
    _FuriHalSdErrorTxUnderrun,
    _FuriHalSdErrorRxOverrun,
    _FuriHalSdErrorAddrMisaligned,
    _FuriHalSdErrorBlockLenErr,
    _FuriHalSdErrorEraseSeqErr,
    _FuriHalSdErrorBadEraseParam,
    _FuriHalSdErrorWriteProtViolation,
    _FuriHalSdErrorLockUnlockFailed,
    _FuriHalSdErrorComCrcFailed,
    _FuriHalSdErrorIllegalCmd,
    _FuriHalSdErrorCardEccFailed,
    _FuriHalSdErrorCcErr,
    _FuriHalSdErrorGeneralUnknownErr,
    _FuriHalSdErrorStreamReadUnderrun,
    _FuriHalSdErrorStreamWriteOverrun,
    _FuriHalSdErrorCidCsdOverwrite,
    _FuriHalSdErrorWpEraseSkip,
    _FuriHalSdErrorCardEccDisabled,
    _FuriHalSdErrorEraseReset,
    _FuriHalSdErrorAkeSeqErr,
    _FuriHalSdErrorInvalidVoltRange,
    _FuriHalSdErrorAddrOutOfRange,
    _FuriHalSdErrorRequestNotApplicable,
    _FuriHalSdErrorParam,
    _FuriHalSdErrorUnsupportedFeature,
    _FuriHalSdErrorBusy,
    _FuriHalSdErrorDma,
    _FuriHalSdErrorTimeout,
    _FuriHalSdErrorCardAbsent,
    _FuriHalSdErrorOther,
} _FuriHalSdError;

typedef struct FuriHalSdError {
    _FuriHalSdError e;
} FuriHalSdError;

#define FuriHalSdErrorNone (FuriHalSdError){0}
#define FuriHalSdErrorDataCrcFail (FuriHalSdError){_FuriHalSdErrorDataCrcFail}
#define FuriHalSdErrorDataTimeout (FuriHalSdError){_FuriHalSdErrorDataTimeout}
#define FuriHalSdErrorTxUnderrun (FuriHalSdError){_FuriHalSdErrorTxUnderrun}
#define FuriHalSdErrorRxOverrun (FuriHalSdError){_FuriHalSdErrorRxOverrun}
#define FuriHalSdErrorAddrMisaligned (FuriHalSdError){_FuriHalSdErrorAddrMisaligned}
#define FuriHalSdErrorBlockLenErr (FuriHalSdError){_FuriHalSdErrorBlockLenErr}
#define FuriHalSdErrorEraseSeqErr (FuriHalSdError){_FuriHalSdErrorEraseSeqErr}
#define FuriHalSdErrorBadEraseParam (FuriHalSdError){_FuriHalSdErrorBadEraseParam}
#define FuriHalSdErrorWriteProtViolation (FuriHalSdError){_FuriHalSdErrorWriteProtViolation}
#define FuriHalSdErrorLockUnlockFailed (FuriHalSdError){_FuriHalSdErrorLockUnlockFailed}
#define FuriHalSdErrorComCrcFailed (FuriHalSdError){_FuriHalSdErrorComCrcFailed}
#define FuriHalSdErrorIllegalCmd (FuriHalSdError){_FuriHalSdErrorIllegalCmd}
#define FuriHalSdErrorCardEccFailed (FuriHalSdError){_FuriHalSdErrorCardEccFailed}
#define FuriHalSdErrorCcErr (FuriHalSdError){_FuriHalSdErrorCcErr}
#define FuriHalSdErrorGeneralUnknownErr (FuriHalSdError){_FuriHalSdErrorGeneralUnknownErr}
#define FuriHalSdErrorStreamReadUnderrun (FuriHalSdError){_FuriHalSdErrorStreamReadUnderrun}
#define FuriHalSdErrorStreamWriteOverrun (FuriHalSdError){_FuriHalSdErrorStreamWriteOverrun}
#define FuriHalSdErrorCidCsdOverwrite (FuriHalSdError){_FuriHalSdErrorCidCsdOverwrite}
#define FuriHalSdErrorWpEraseSkip (FuriHalSdError){_FuriHalSdErrorWpEraseSkip}
#define FuriHalSdErrorCardEccDisabled (FuriHalSdError){_FuriHalSdErrorCardEccDisabled}
#define FuriHalSdErrorEraseReset (FuriHalSdError){_FuriHalSdErrorEraseReset}
#define FuriHalSdErrorAkeSeqErr (FuriHalSdError){_FuriHalSdErrorAkeSeqErr}
#define FuriHalSdErrorInvalidVoltRange (FuriHalSdError){_FuriHalSdErrorInvalidVoltRange}
#define FuriHalSdErrorAddrOutOfRange (FuriHalSdError){_FuriHalSdErrorAddrOutOfRange}
#define FuriHalSdErrorRequestNotApplicable (FuriHalSdError){_FuriHalSdErrorRequestNotApplicable}
#define FuriHalSdErrorParam (FuriHalSdError){_FuriHalSdErrorParam}
#define FuriHalSdErrorUnsupportedFeature (FuriHalSdError){_FuriHalSdErrorUnsupportedFeature}
#define FuriHalSdErrorBusy (FuriHalSdError){_FuriHalSdErrorBusy}
#define FuriHalSdErrorDma (FuriHalSdError){_FuriHalSdErrorDma}
#define FuriHalSdErrorTimeout (FuriHalSdError){_FuriHalSdErrorTimeout}
#define FuriHalSdErrorCardAbsent (FuriHalSdError){_FuriHalSdErrorCardAbsent}
#define FuriHalSdErrorOther (FuriHalSdError){_FuriHalSdErrorOther}

bool furi_hal_sdmmc_error_is_ok(FuriHalSdError e);

typedef void (*FuriHalSdMmcPresentCallback)(void* context);

void furi_hal_sdmmc_init(bool have_rtos);

FURI_CHECK_RETURN
FuriHalSdError furi_hal_sdmmc_init_card(void);

void furi_hal_sdmmc_deinit_card(void);

FURI_CHECK_RETURN
FuriHalSdError furi_hal_sdmmc_read_blocks(
    uint8_t* buffer,
    uint32_t address,
    uint32_t count,
    size_t timeout_ms_per_block);

FURI_CHECK_RETURN
FuriHalSdError furi_hal_sdmmc_write_blocks(
    const uint8_t* buffer,
    uint32_t address,
    uint32_t count,
    size_t timeout_ms);

FURI_CHECK_RETURN
FuriHalSdError furi_hal_sdmmc_get_card_info(FuriHalSdInfo* info);

bool furi_hal_sd_alive(void);
