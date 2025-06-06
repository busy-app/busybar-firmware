#pragma once
#include <stdint.h>

typedef enum {
    FuriHalNvmFlagDebug,
    FuriHalNvmFlagDetailedFilename,
    FuriHalNvmFlagStorageFormatInternal,
    FuriHalNvmFlagHandOrient
} FuriHalRtcFlag;

typedef enum {
    FuriHalNvmBootModeNormal, /**< Normal boot mode, default value */
} FuriHalRtcBootMode;

/** Reset ALL RTC registers content */
void furi_hal_nvm_reset_registers(void);

/** Set RTC Flag
 *
 * @param[in]  flag  The flag to set
 */
void furi_hal_nvm_set_flag(FuriHalRtcFlag flag);

/** Check if RTC Flag is set
 *
 * @param[in]  flag  The flag to check
 *
 * @return     true if set
 */
bool furi_hal_nvm_is_flag_set(FuriHalRtcFlag flag);

/** Get RTC boot mode
 *
 * @return     The RTC boot mode.
 */
FuriHalRtcBootMode furi_hal_nvm_get_boot_mode(void);
